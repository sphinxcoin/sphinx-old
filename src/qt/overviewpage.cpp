#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "util.h"
#include "clientmodel.h"
#include "stashedsend.h"
#include "stashedsendconfig.h"
#include "walletmodel.h"
#include "sphinxcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QScrollArea>
#include <QScroller>
#include <QSettings>
#include <QTimer>

#define DECORATSPHX_SIZE 64
#define ICON_OFFSET 16
#define NUM_ITEMS 6

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(SphinxcoinUnits::SPHX)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        mainRect.moveLeft(ICON_OFFSET);
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATSPHX_SIZE, DECORATSPHX_SIZE));
        int xspace = DECORATSPHX_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace - ICON_OFFSET, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(qVariantCanConvert<QColor>(value))
        {
            foreground = qvariant_cast<QColor>(value);
        }

        painter->setPen(foreground);
        QRect boundingRect;
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address, &boundingRect);

        if (index.data(TransactionTableModel::WatchonlyRole).toBool())
        {
            QIcon iconWatchonly = qvariant_cast<QIcon>(index.data(TransactionTableModel::WatchonlyDecorationRole));
            QRect watchonlyRect(boundingRect.right() + 5, mainRect.top()+ypad+halfheight, 16, halfheight);
            iconWatchonly.paint(painter, watchonlyRect);
        }

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = SphinxcoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATSPHX_SIZE, DECORATSPHX_SIZE);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    clientModel(0),
    walletModel(0),
    currentBalance(-1),
    currentStake(-1),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    currentWatchOnlyBalance(-1),
    currentWatchOnlyStake(-1),
    currentWatchUnconfBalance(-1),
    currentWatchImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    ui->setupUi(this);

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATSPHX_SIZE, DECORATSPHX_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATSPHX_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listTransactions->setMinimumWidth(300);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    ui->labelStashedsendSyncStatus->setText("(" + tr("out of sync") + ")");
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");

    fLiteMode = GetBoolArg("-litemode", false);
    if(fLiteMode) {
        ui->frameStashedsend->setVisible(false);
    } else {
        if(fMasterNode) {
            ui->toggleStashedsend->setText("(" + tr("Disabled") + ")");
            ui->stashedsendAuto->setText("(" + tr("Disabled") + ")");
            ui->stashedsendReset->setText("(" + tr("Disabled") + ")");
            ui->frameStashedsend->setEnabled(false);
        } else {
            if(!fEnableStashedsend) {
                ui->toggleStashedsend->setText(tr("Start Darksend Mixing"));
            } else {
                ui->toggleStashedsend->setText(tr("Stop Darksend Mixing"));
            }
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(stashedSendStatus()));
            if(!GetBoolArg("-reindexaddr", false))
                timer->start(60000);
        }
    }

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
}

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    if(!fLiteMode && !fMasterNode) disconnect(timer, SIGNAL(timeout()), this, SLOT(stashedSendStatus()));
    delete ui;
}

void OverviewPage::setBalance(const CAmount& balance, const CAmount& stake, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, const CAmount& anonymizedBalance, const CAmount& watchOnlyBalance, const CAmount& watchOnlyStake, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    currentAnonymizedBalance = anonymizedBalance;
    currentWatchOnlyBalance = watchOnlyBalance;
    currentWatchOnlyStake = watchOnlyStake;
    currentWatchUnconfBalance = watchUnconfBalance;
    currentWatchImmatureBalance = watchImmatureBalance;
    ui->labelBalance->setText(SphinxcoinUnits::formatWithUnit(nDisplayUnit, balance));
    ui->labelStake->setText(SphinxcoinUnits::formatWithUnit(nDisplayUnit, stake));
    ui->labelUnconfirmed->setText(SphinxcoinUnits::formatWithUnit(nDisplayUnit, unconfirmedBalance));
    ui->labelImmature->setText(SphinxcoinUnits::formatWithUnit(nDisplayUnit, immatureBalance));
    ui->labelAnonymized->setText(SphinxcoinUnits::formatWithUnit(nDisplayUnit, anonymizedBalance));
    ui->labelTotal->setText(SphinxcoinUnits::formatWithUnit(nDisplayUnit, balance + stake + unconfirmedBalance + immatureBalance));
    ui->labelWatchAvailable->setText(SphinxcoinUnits::floorWithUnit(nDisplayUnit, watchOnlyBalance));
    ui->labelWatchStake->setText(SphinxcoinUnits::floorWithUnit(nDisplayUnit, watchOnlyStake));
    ui->labelWatchPending->setText(SphinxcoinUnits::floorWithUnit(nDisplayUnit, watchUnconfBalance));
    ui->labelWatchImmature->setText(SphinxcoinUnits::floorWithUnit(nDisplayUnit, watchImmatureBalance));
    ui->labelWatchTotal->setText(SphinxcoinUnits::floorWithUnit(nDisplayUnit, watchOnlyBalance + watchOnlyStake + watchUnconfBalance + watchImmatureBalance));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    bool showWatchOnlyImmature = watchImmatureBalance != 0;

    // for symmetry reasons also show immature label when the watch-only one is shown
    ui->labelImmature->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelImmatureText->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelWatchImmature->setVisible(showWatchOnlyImmature); // show watch-only immature balance

    updateStashedsendProgress();

    static int cachedTxLocks = 0;

    if(cachedTxLocks != nCompleteTXLocks) {
        cachedTxLocks = nCompleteTXLocks;
        ui->listTransactions->update();
    }
}

// show/hide watch-only labels
void OverviewPage::updateWatchOnlyLabels(bool showWatchOnly)
{
    ui->labelSpendable->setVisible(showWatchOnly);      // show spendable label (only when watch-only is active)
    ui->labelWatchonly->setVisible(showWatchOnly);      // show watch-only label
    ui->lineWatchBalance->setVisible(showWatchOnly);    // show watch-only balance separator line
    ui->labelWatchStake->setVisible(showWatchOnly);    // show watch-only balance separator line
    ui->labelWatchAvailable->setVisible(showWatchOnly); // show watch-only available balance
    ui->labelWatchPending->setVisible(showWatchOnly);   // show watch-only pending balance
    ui->labelWatchTotal->setVisible(showWatchOnly);     // show watch-only total balance

    if (!showWatchOnly) {
        ui->labelWatchImmature->hide();
    }
    else {
        ui->labelBalance->setIndent(20);
        ui->labelStake->setIndent(20);
        ui->labelUnconfirmed->setIndent(20);
        ui->labelImmature->setIndent(20);
        ui->labelTotal->setIndent(20);
    }
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        // Show warning if this is a prerelease version
        connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
        updateAlerts(model->getStatusBarWarnings());
    }
}

void OverviewPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Status, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance(), model->getAnonymizedBalance(),
                   model->getWatchBalance(), model->getWatchStake(), model->getWatchUnconfirmedBalance(), model->getWatchImmatureBalance());
        connect(model, SIGNAL(balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)), this, SLOT(setBalance(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

        connect(ui->stashedsendAuto, SIGNAL(clicked()), this, SLOT(stashedsendAuto()));
        connect(ui->stashedsendReset, SIGNAL(clicked()), this, SLOT(stashedsendReset()));
        connect(ui->toggleStashedsend, SIGNAL(clicked()), this, SLOT(toggleStashedsend()));
        updateWatchOnlyLabels(model->haveWatchOnly());
        connect(model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyLabels(bool)));
    }

    // update the display unit, to not use the default ("SPHX")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(walletModel && walletModel->getOptionsModel())
    {

        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if(currentBalance != -1)
            setBalance(currentBalance, currentStake, currentUnconfirmedBalance, currentImmatureBalance, currentAnonymizedBalance,
                       currentWatchOnlyBalance, currentWatchOnlyStake, currentWatchUnconfBalance, currentWatchImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = nDisplayUnit;

        ui->listTransactions->update();
    }
}

void OverviewPage::updateAlerts(const QString &warnings)
{
    this->ui->labelAlerts->setVisible(!warnings.isEmpty());
    this->ui->labelAlerts->setText(warnings);
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
    ui->labelStashedsendSyncStatus->setVisible(fShow);
    ui->labelTransactionsStatus->setVisible(fShow);
}



void OverviewPage::updateStashedsendProgress()
{
    if(!stashedSendPool.IsBlockchainSynced() || ShutdownRequested()) return;

    if(!pwalletMain) return;

    QString strAmountAndRounds;
    QString strAnonymizeSphinxcoinAmount = SphinxcoinUnits::formatHtmlWithUnit(nDisplayUnit, nAnonymizeSphinxcoinAmount * COIN, false, SphinxcoinUnits::separatorAlways);

    if(currentBalance == 0)
    {
        ui->stashedsendProgress->setValue(0);
        ui->stashedsendProgress->setToolTip(tr("No inputs detected"));
        // when balance is zero just show info from settings
        strAnonymizeSphinxcoinAmount = strAnonymizeSphinxcoinAmount.remove(strAnonymizeSphinxcoinAmount.indexOf("."), SphinxcoinUnits::decimals(nDisplayUnit) + 1);
        strAmountAndRounds = strAnonymizeSphinxcoinAmount + " / " + tr("%n Rounds", "", nStashedsendRounds);

        ui->labelAmountRounds->setToolTip(tr("No inputs detected"));
        ui->labelAmountRounds->setText(strAmountAndRounds);
        return;
    }

    CAmount nDenominatedConfirmedBalance;
    CAmount nDenominatedUnconfirmedBalance;
    CAmount nAnonymizableBalance;
    CAmount nNormalizedAnonymizedBalance;
    double nAverageAnonymizedRounds;

    {
        TRY_LOCK(cs_main, lockMain);
        if(!lockMain) return;

        nDenominatedConfirmedBalance = pwalletMain->GetDenominatedBalance();
        nDenominatedUnconfirmedBalance = pwalletMain->GetDenominatedBalance(true);
        nAnonymizableBalance = pwalletMain->GetAnonymizableBalance();
        nNormalizedAnonymizedBalance = pwalletMain->GetNormalizedAnonymizedBalance();
        nAverageAnonymizedRounds = pwalletMain->GetAverageAnonymizedRounds();
    }

    //Get the anon threshold
    CAmount nMaxToAnonymize = nAnonymizableBalance + currentAnonymizedBalance + nDenominatedUnconfirmedBalance;

    // If it's more than the anon threshold, limit to that.
    if(nMaxToAnonymize > nAnonymizeSphinxcoinAmount*COIN) nMaxToAnonymize = nAnonymizeSphinxcoinAmount*COIN;

    if(nMaxToAnonymize == 0) return;

    if(nMaxToAnonymize >= nAnonymizeSphinxcoinAmount * COIN) {
        ui->labelAmountRounds->setToolTip(tr("Found enough compatible inputs to anonymize %1")
                                          .arg(strAnonymizeSphinxcoinAmount));
        strAnonymizeSphinxcoinAmount = strAnonymizeSphinxcoinAmount.remove(strAnonymizeSphinxcoinAmount.indexOf("."), SphinxcoinUnits::decimals(nDisplayUnit) + 1);
        strAmountAndRounds = strAnonymizeSphinxcoinAmount + " / " + tr("%n Rounds", "", nStashedsendRounds);
    } else {
        QString strMaxToAnonymize = SphinxcoinUnits::formatHtmlWithUnit(nDisplayUnit, nMaxToAnonymize, false, SphinxcoinUnits::separatorAlways);
        ui->labelAmountRounds->setToolTip(tr("Not enough compatible inputs to anonymize <span style='color:red;'>%1</span>,<br>"
                                             "will anonymize <span style='color:red;'>%2</span> instead")
                                          .arg(strAnonymizeSphinxcoinAmount)
                                          .arg(strMaxToAnonymize));
        strMaxToAnonymize = strMaxToAnonymize.remove(strMaxToAnonymize.indexOf("."), SphinxcoinUnits::decimals(nDisplayUnit) + 1);
        strAmountAndRounds = "<span style='color:red;'>" +
                             QString(SphinxcoinUnits::factor(nDisplayUnit) == 1 ? "" : "~") + strMaxToAnonymize +
                             " / " + tr("%n Rounds", "", nStashedsendRounds) + "</span>";
    }
    ui->labelAmountRounds->setText(strAmountAndRounds);

    // calculate parts of the progress, each of them shouldn't be higher than 1
    // progress of denominating
    float denomPart = 0;
    // mixing progress of denominated balance
    float anonNormPart = 0;
    // completeness of full amount anonimization
    float anonFullPart = 0;

    CAmount denominatedBalance = nDenominatedConfirmedBalance + nDenominatedUnconfirmedBalance;
    denomPart = (float)denominatedBalance / nMaxToAnonymize;
    denomPart = denomPart > 1 ? 1 : denomPart;
    denomPart *= 100;

    anonNormPart = (float)nNormalizedAnonymizedBalance / nMaxToAnonymize;
    anonNormPart = anonNormPart > 1 ? 1 : anonNormPart;
    anonNormPart *= 100;

    anonFullPart = (float)currentAnonymizedBalance / nMaxToAnonymize;
    anonFullPart = anonFullPart > 1 ? 1 : anonFullPart;
    anonFullPart *= 100;

    // apply some weights to them ...
    float denomWeight = 1;
    float anonNormWeight = nStashedsendRounds;
    float anonFullWeight = 2;
    float fullWeight = denomWeight + anonNormWeight + anonFullWeight;
    // ... and calculate the whole progress
    float denomPartCalc = ceilf((denomPart * denomWeight / fullWeight) * 100) / 100;
    float anonNormPartCalc = ceilf((anonNormPart * anonNormWeight / fullWeight) * 100) / 100;
    float anonFullPartCalc = ceilf((anonFullPart * anonFullWeight / fullWeight) * 100) / 100;
    float progress = denomPartCalc + anonNormPartCalc + anonFullPartCalc;
    if(progress >= 100) progress = 100;

    ui->stashedsendProgress->setValue(progress);

    QString strToolPip = ("<b>" + tr("Overall progress") + ": %1%</b><br/>" +
                          tr("Denominated") + ": %2%<br/>" +
                          tr("Mixed") + ": %3%<br/>" +
                          tr("Anonymized") + ": %4%<br/>" +
                          tr("Denominated inputs have %5 of %n rounds on average", "", nStashedsendRounds))
                         .arg(progress).arg(denomPart).arg(anonNormPart).arg(anonFullPart)
                         .arg(nAverageAnonymizedRounds);
    ui->stashedsendProgress->setToolTip(strToolPip);
}


void OverviewPage::stashedSendStatus()
{
    static int64_t nLastDSProgressBlockTime = 0;

    int nBestHeight = pindexBest->nHeight;

    // we we're processing more then 1 block per second, we'll just leave
    if(((nBestHeight - stashedSendPool.cachedNumBlocks) / (GetTimeMillis() - nLastDSProgressBlockTime + 1) > 1)) return;
    nLastDSProgressBlockTime = GetTimeMillis();

    if(!fEnableStashedsend) {
        if(nBestHeight != stashedSendPool.cachedNumBlocks)
        {
            stashedSendPool.cachedNumBlocks = nBestHeight;
            updateStashedsendProgress();

            ui->stashedsendEnabled->setText(tr("Disabled"));
            ui->stashedsendStatus->setText("");
            ui->toggleStashedsend->setText(tr("Start Darksend Mixing"));
        }

        return;
    }

    // check stashedsend status and unlock if needed
    if(nBestHeight != stashedSendPool.cachedNumBlocks)
    {
        // Balance and number of transactions might have changed
        stashedSendPool.cachedNumBlocks = nBestHeight;

        updateStashedsendProgress();

        ui->stashedsendEnabled->setText(tr("Enabled"));
    }

    QString strStatus = QString(stashedSendPool.GetStatus().c_str());

    QString s = tr("Last Darksend message:\n") + strStatus;

    if(s != ui->stashedsendStatus->text())
        LogPrintf("Last Darksend message: %s\n", strStatus.toStdString());

    ui->stashedsendStatus->setText(s);

    if(stashedSendPool.sessionDenom == 0) {
        ui->labelSubmittedDenom->setText(tr("N/A"));
    } else {
        std::string out;
        stashedSendPool.GetDenominationsToString(stashedSendPool.sessionDenom, out);
        QString s2(out.c_str());
        ui->labelSubmittedDenom->setText(s2);
    }

    // Get StashedSend Denomination Status
}

void OverviewPage::stashedsendAuto() {
    stashedSendPool.DoAutomaticDenominating();
}

void OverviewPage::stashedsendReset() {
    stashedSendPool.Reset();
    stashedSendStatus();

    QMessageBox::warning(this, tr("Darksend"),
                         tr("Darksend was successfully reset."),
                         QMessageBox::Ok, QMessageBox::Ok);
}

void OverviewPage::toggleStashedsend() {
    QMessageBox::StandardButton askIfAGoodIdea;
    askIfAGoodIdea = QMessageBox::question(this, tr("StashedSend Mixing"), 
												 tr("StashedSend is not for production networks yet! Please continue at your own risk! Any loss of coins will be the sole liability of the user, would you like to continue?"),
                                           QMessageBox::Yes|QMessageBox::No);

    if (askIfAGoodIdea == QMessageBox::Yes) {
        QSettings settings;
        // Popup some information on first mixing
        QString hasMixed = settings.value("hasMixed").toString();
        if(hasMixed.isEmpty()) {
            QMessageBox::information(this, tr("Darksend"),
                                     tr("If you don't want to see internal Darksend fees/transactions select \"Received By\" as Type on the \"Transactions\" tab."),
                                     QMessageBox::Ok, QMessageBox::Ok);
            settings.setValue("hasMixed", "hasMixed");
        }

        if(!fEnableStashedsend) {
            int64_t balance = currentBalance;
            float minAmount = 1.49 * COIN;
            if(balance < minAmount) {
                QString strMinAmount(SphinxcoinUnits::formatWithUnit(nDisplayUnit, minAmount));
                QMessageBox::warning(this, tr("Darksend"),
                                     tr("Darksend requires at least %1 to use.").arg(strMinAmount),
                                     QMessageBox::Ok, QMessageBox::Ok);
                return;
            }

            // if wallet is locked, ask for a passphrase
            if (walletModel->getEncryptionStatus() == WalletModel::Locked)
            {
                WalletModel::UnlockContext ctx(walletModel->requestUnlock());
                if(!ctx.isValid())
                {
                    //unlock was cancelled
                    stashedSendPool.cachedNumBlocks = std::numeric_limits<int>::max();
                    QMessageBox::warning(this, tr("Darksend"),
                                         tr("Wallet is locked and user declined to unlock. Disabling Darksend."),
                                         QMessageBox::Ok, QMessageBox::Ok);
                    if (fDebug) LogPrintf("Wallet is locked and user declined to unlock. Disabling Darksend.\n");
                    return;
                }
            }

        }

        fEnableStashedsend = !fEnableStashedsend;
        stashedSendPool.cachedNumBlocks = std::numeric_limits<int>::max();

        if(!fEnableStashedsend) {
            ui->toggleStashedsend->setText(tr("Start Darksend Mixing"));
            stashedSendPool.UnlockCoins();
        } else {
            ui->toggleStashedsend->setText(tr("Stop Darksend Mixing"));

            /* show stashedsend configuration if client has defaults set */

            if(nAnonymizeSphinxcoinAmount == 0) {
                StashedsendConfig dlg(this);
                dlg.setModel(walletModel);
                dlg.exec();
            }

        }
    } else {
        // Keep it blank...
    }
}
