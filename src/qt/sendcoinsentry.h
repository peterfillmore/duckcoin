// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITBREADCRUMB_QT_SENDBREADCRUMBSENTRY_H
#define BITBREADCRUMB_QT_SENDBREADCRUMBSENTRY_H

#include "walletmodel.h"

#include <QStackedWidget>

class WalletModel;

namespace Ui {
    class SendBreadcrumbsEntry;
}

/**
 * A single entry in the dialog for sending bitcoins.
 * Stacked widget, with different UIs for payment requests
 * with a strong payee identity.
 */
class SendBreadcrumbsEntry : public QStackedWidget
{
    Q_OBJECT

public:
    explicit SendBreadcrumbsEntry(QWidget *parent = 0);
    ~SendBreadcrumbsEntry();

    void setModel(WalletModel *model);
    bool validate();
    SendBreadcrumbsRecipient getValue();

    /** Return whether the entry is still empty and unedited */
    bool isClear();

    void setValue(const SendBreadcrumbsRecipient &value);
    void setAddress(const QString &address);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases
     *  (issue https://bugreports.qt-project.org/browse/QTBUG-10907).
     */
    QWidget *setupTabChain(QWidget *prev);

    void setFocus();

public slots:
    void clear();

signals:
    void removeEntry(SendBreadcrumbsEntry *entry);
    void payAmountChanged();

private slots:
    void deleteClicked();
    void on_payTo_textChanged(const QString &address);
    void on_addressBookButton_clicked();
    void on_pasteButton_clicked();
    void updateDisplayUnit();

private:
    SendBreadcrumbsRecipient recipient;
    Ui::SendBreadcrumbsEntry *ui;
    WalletModel *model;

    bool updateLabel(const QString &address);
};

#endif // BITBREADCRUMB_QT_SENDBREADCRUMBSENTRY_H
