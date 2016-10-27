// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITBREADCRUMB_QT_WALLETMODELTRANSACTION_H
#define BITBREADCRUMB_QT_WALLETMODELTRANSACTION_H

#include "walletmodel.h"

#include <QObject>

class SendBreadcrumbsRecipient;

class CReserveKey;
class CWallet;
class CWalletTx;

/** Data model for a walletmodel transaction. */
class WalletModelTransaction
{
public:
    explicit WalletModelTransaction(const QList<SendBreadcrumbsRecipient> &recipients);
    ~WalletModelTransaction();

    QList<SendBreadcrumbsRecipient> getRecipients();

    CWalletTx *getTransaction();
    unsigned int getTransactionSize();

    void setTransactionFee(const CAmount& newFee);
    CAmount getTransactionFee();

    CAmount getTotalTransactionAmount();

    void newPossibleKeyChange(CWallet *wallet);
    CReserveKey *getPossibleKeyChange();

private:
    const QList<SendBreadcrumbsRecipient> recipients;
    CWalletTx *walletTransaction;
    CReserveKey *keyChange;
    CAmount fee;
};

#endif // BITBREADCRUMB_QT_WALLETMODELTRANSACTION_H
