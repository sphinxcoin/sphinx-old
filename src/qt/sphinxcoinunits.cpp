// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sphinxcoinunits.h"
#include "main.h"

#include <QSettings>
#include <QStringList>

SphinxcoinUnits::SphinxcoinUnits(QObject *parent):
        QAbstractListModel(parent),
        unitlist(availableUnits())
{
}

QList<SphinxcoinUnits::Unit> SphinxcoinUnits::availableUnits()
{
    QList<SphinxcoinUnits::Unit> unitlist;
    unitlist.append(SPHX);
    unitlist.append(mSPHX);
    unitlist.append(uSPHX);
    return unitlist;
}

bool SphinxcoinUnits::valid(int unit)
{
    switch(unit)
    {
    case SPHX:
    case mSPHX:
    case uSPHX:
        return true;
    default:
        return false;
    }
}

QString SphinxcoinUnits::name(int unit)
{
    switch(unit)
    {
    case SPHX: return QString("SPHX");
    case mSPHX: return QString("mSPHX");
    case uSPHX: return QString::fromUtf8("μSPHX");
    default: return QString("???");
    }
}

QString SphinxcoinUnits::description(int unit)
{
    switch(unit)
    {
    case SPHX: return QString("SPHXs");
    case mSPHX: return QString("Milli-SPHXs (1 / 1,000)");
    case uSPHX: return QString("Micro-SPHXs (1 / 1,000,000)");
    default: return QString("???");
    }
}

qint64 SphinxcoinUnits::factor(int unit)
{
    switch(unit)
    {
    case SPHX:  return 100000000;
    case mSPHX: return 100000;
    case uSPHX: return 100;
    default:   return 100000000;
    }
}

int SphinxcoinUnits::amountDigits(int unit)
{
    switch(unit)
    {
    case SPHX: return 8; // 21,000,000 (# digits, without commas)
    case mSPHX: return 11; // 21,000,000,000
    case uSPHX: return 14; // 21,000,000,000,000
    default: return 0;
    }
}

int SphinxcoinUnits::decimals(int unit)
{
    switch(unit)
    {
    case SPHX: return 8;
    case mSPHX: return 5;
    case uSPHX: return 2;
    default: return 0;
    }
}

QString SphinxcoinUnits::format(int unit, const CAmount& nIn, bool fPlus, SeparatorStyle separators)
{
    // Note: not using straight sprintf here because we do NOT want
    // localized number formatting.
    if(!valid(unit))
        return QString(); // Refuse to format invalid unit
    qint64 n = (qint64)nIn;
    qint64 coin = factor(unit);
    int num_decimals = decimals(unit);
    qint64 n_abs = (n > 0 ? n : -n);
    qint64 quotient = n_abs / coin;
    qint64 remainder = n_abs % coin;
    QString quotient_str = QString::number(quotient);
    QString remainder_str = QString::number(remainder).rightJustified(num_decimals, '0');

    // Right-trim excess zeros after the decimal point
    int nTrim = 0;
    for (int i = remainder_str.size()-1; i>=2 && (remainder_str.at(i) == '0'); --i)
        ++nTrim;
    remainder_str.chop(nTrim);

    if (n < 0)
        quotient_str.insert(0, '-');
    else if (fPlus && n > 0)
        quotient_str.insert(0, '+');
    return quotient_str + QString(".") + remainder_str;
}

// TODO: Review all remaining calls to SphinxcoinUnits::formatWithUnit to
// TODO: determine whether the output is used in a plain text context
// TODO: or an HTML context (and replace with
// TODO: SphinxcoinoinUnits::formatHtmlWithUnit in the latter case). Hopefully
// TODO: there aren't instances where the result could be used in
// TODO: either context.

// NOTE: Using formatWithUnit in an HTML context risks wrapping
// quantities at the thousands separator. More subtly, it also results
// in a standard space rather than a thin space, due to a bug in Qt's
// XML whitespace canonicalisation
//
// Please take care to use formatHtmlWithUnit instead, when
// appropriate.

QString SphinxcoinUnits::formatWithUnit(int unit, const CAmount& amount, bool plussign, SeparatorStyle separators)
{
    return format(unit, amount, plussign, separators) + QString(" ") + name(unit);
}

QString SphinxcoinUnits::formatHtmlWithUnit(int unit, const CAmount& amount, bool plussign, SeparatorStyle separators)
{
    QString str(formatWithUnit(unit, amount, plussign, separators));
    str.replace(QChar(THIN_SP_CP), QString(THIN_SP_HTML));
    return QString("<span style='white-space: nowrap;'>%1</span>").arg(str);
}

QString SphinxcoinUnits::floorWithUnit(int unit, const CAmount& amount, bool plussign, SeparatorStyle separators)
{
    QSettings settings;
    int digits = settings.value("digits").toInt();

    QString result = format(unit, amount, plussign, separators);
    if(decimals(unit) > digits) result.chop(decimals(unit) - digits);

    return result + QString(" ") + name(unit);
}

QString SphinxcoinUnits::floorHtmlWithUnit(int unit, const CAmount& amount, bool plussign, SeparatorStyle separators)
{
    QString str(floorWithUnit(unit, amount, plussign, separators));
    str.replace(QChar(THIN_SP_CP), QString(THIN_SP_HTML));
    return QString("<span style='white-space: nowrap;'>%1</span>").arg(str);
}

bool SphinxcoinUnits::parse(int unit, const QString &value, CAmount *val_out)
{
    if(!valid(unit) || value.isEmpty())
        return false; // Refuse to parse invalid unit or empty string
    int num_decimals = decimals(unit);

    // Ignore spaces and thin spaces when parsing
    QStringList parts = removeSpaces(value).split(".");

    if(parts.size() > 2)
    {
        return false; // More than one dot
    }
    QString whole = parts[0];
    QString decimals;

    if(parts.size() > 1)
    {
        decimals = parts[1];
    }
    if(decimals.size() > num_decimals)
    {
        return false; // Exceeds max precision
    }
    bool ok = false;
    QString str = whole + decimals.leftJustified(num_decimals, '0');

    if(str.size() > 18)
    {
        return false; // Longer numbers will exceed 63 bits
    }
    CAmount retvalue = str.toLongLong(&ok);
    if(val_out)
    {
        *val_out = retvalue;
    }
    return ok;
}

QString SphinxcoinUnits::getAmountColumnTitle(int unit)
{
    QString amountTitle = QObject::tr("Amount");
    if (SphinxcoinUnits::valid(unit))
    {
        amountTitle += " ("+SphinxcoinUnits::name(unit) + ")";
    }
    return amountTitle;
}

int SphinxcoinUnits::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return unitlist.size();
}

QVariant SphinxcoinUnits::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row >= 0 && row < unitlist.size())
    {
        Unit unit = unitlist.at(row);
        switch(role)
        {
        case Qt::EditRole:
        case Qt::DisplayRole:
            return QVariant(name(unit));
        case Qt::ToolTipRole:
            return QVariant(description(unit));
        case UnitRole:
            return QVariant(static_cast<int>(unit));
        }
    }
    return QVariant();
}

CAmount SphinxcoinUnits::maxMoney()
{
    return MAX_MONEY;
}
