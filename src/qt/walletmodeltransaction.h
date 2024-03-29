// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPHX_QT_WALLETMODELTRANSACTSPHX_H
#define SPHX_QT_WALLETMODELTRANSACTSPHX_H

#include "walletmodel.h"
#include "util.h"
#include "amount.h"

#include <QObject>

class SendCoinsRecipient;

class CReserveKey;
class CWallet;
class CWalletTx;

/** Data model for a walletmodel transaction. */
class WalletModelTransaction
{
public:
    explicit WalletModelTransaction(const QList<SendCoinsRecipient> &recipients);
    ~WalletModelTransaction();

    QList<SendCoinsRecipient> getRecipients();

    CWalletTx *getTransaction();
    unsigned int getTransactionSize();

    void setTransactionFee(const CAmount& newFee);
    CAmount getTransactionFee();

    CAmount getTotalTransactionAmount();

    void newPossibleKeyChange(CWallet *wallet);
    CReserveKey *getPossibleKeyChange();

private:
    const QList<SendCoinsRecipient> recipients;
    CWalletTx *walletTransaction;
    CReserveKey *keyChange;
    CAmount fee;
};

#endif // SPHX_QT_WALLETMODELTRANSACTSPHX_H
