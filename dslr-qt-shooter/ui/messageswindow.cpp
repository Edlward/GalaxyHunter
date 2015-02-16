/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "messageswindow.h"
#include "ui_messageswindow.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QStringListModel>
#include <QStandardItemModel>

class MessagesWindow::Private {
public:
  Private(MessagesWindow* q);
  std::unique_ptr<Ui::MessagesWindow> ui;
  std::unique_ptr<QSortFilterProxyModel> logsModel;
  QStringListModel filterSender;
  QStringListModel filterType;
private:
  MessagesWindow *q;
};

MessagesWindow::Private::Private(MessagesWindow* q)
  : q(q), logsModel(new QSortFilterProxyModel), ui(new Ui::MessagesWindow)
{
}


MessagesWindow::~MessagesWindow()
{
}

MessagesWindow::MessagesWindow(QAbstractItemModel *logsModel, QWidget* parent) : QWidget(parent), d(new Private{this})
{
  d->ui->setupUi(this);
  d->logsModel->setSourceModel(logsModel);
  d->ui->logs->setModel(d->logsModel.get());
  connect(d->ui->clearLogs, &QPushButton::clicked, [=]{ logsModel->removeRows(0, logsModel->rowCount()); });
  d->logsModel->setDynamicSortFilter(true);
  d->logsModel->setSortRole(Qt::UserRole+1);
  d->logsModel->sort(0, Qt::DescendingOrder);
  d->logsModel->setFilterKeyColumn(3);
  d->logsModel->setFilterRole(Qt::DisplayRole);
  d->logsModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  connect(d->ui->message, &QLineEdit::textChanged, [=](const QString &t){ d->logsModel->setFilterWildcard(t); });
}
