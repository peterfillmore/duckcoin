// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITBREADCRUMB_QT_BREADCRUMBCONTROLTREEWIDGET_H
#define BITBREADCRUMB_QT_BREADCRUMBCONTROLTREEWIDGET_H

#include <QKeyEvent>
#include <QTreeWidget>

class BreadcrumbControlTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit BreadcrumbControlTreeWidget(QWidget *parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // BITBREADCRUMB_QT_BREADCRUMBCONTROLTREEWIDGET_H
