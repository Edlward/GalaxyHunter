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
  QSortFilterProxyModel logsModel;
  QSortFilterProxyModel filterByType;
  QSortFilterProxyModel filterBySender;
  struct FilterItem {
    QString display;
    QString filter;
  };
  QStandardItemModel type_filters;
  QStandardItemModel sender_filters;
private:
  MessagesWindow *q;
};

MessagesWindow::Private::Private(MessagesWindow* q)
  : q(q), ui(new Ui::MessagesWindow)
{
}


MessagesWindow::~MessagesWindow()
{
}

MessagesWindow::MessagesWindow(QAbstractItemModel *logsModel, QWidget* parent) : QWidget(parent), d(new Private{this})
{
  d->ui->setupUi(this);
  d->logsModel.setSourceModel(logsModel);
  d->filterByType.setSourceModel(&d->logsModel);
  d->filterBySender.setSourceModel(&d->filterByType);
  d->ui->logs->setModel(&d->filterBySender);
  connect(d->ui->clearLogs, &QPushButton::clicked, [=]{ logsModel->removeRows(0, logsModel->rowCount()); });
  d->logsModel.setDynamicSortFilter(true);
  d->logsModel.setSortRole(Qt::UserRole+1);
  d->logsModel.sort(0, Qt::DescendingOrder);
  d->logsModel.setFilterKeyColumn(3);
  d->logsModel.setFilterRole(Qt::DisplayRole);
  d->logsModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
  connect(d->ui->message, &QLineEdit::textChanged, [=](const QString &t){ d->logsModel.setFilterWildcard(t); });
  d->ui->type->setModel(&d->type_filters);
  d->ui->sender->setModel(&d->sender_filters);
  auto filter_item = [=](const QString &text, const QString &filter) { QStandardItem *i = new QStandardItem(text); i->setData(filter); return i; };
  d->type_filters.appendRow(filter_item(tr("All"), QString{}));
  d->sender_filters.appendRow(filter_item(tr("All"), QString{}));
  d->filterByType.setFilterKeyColumn(1);
  d->filterBySender.setFilterKeyColumn(2);
  d->filterByType.setFilterRole(Qt::UserRole+1);
  d->filterBySender.setFilterRole(Qt::UserRole+1);

  auto rows_added = [=](QAbstractItemModel *m, int col, int first, int last, QStandardItemModel &filter_model){
    // TODO: optimize. vector might be unneeded
    std::vector<Private::FilterItem> items;
    for(int i=0; i<filter_model.rowCount(); i++) {
      auto text = filter_model.item(i)->text();
      auto data = filter_model.item(i)->data().toString();
      items.push_back({ text, data });
    }
    for(int i=first; i<=last; i++) {
      auto item_data = m->itemData(m->index(i, col));
      if(std::count_if(items.begin(), items.end(), [=](const Private::FilterItem &f){ return item_data[Qt::UserRole+1].toString() == f.filter;}) > 0) return;
      auto text = item_data[Qt::DisplayRole].toString();
      auto data = item_data[Qt::UserRole+1].toString();
      filter_model.appendRow(filter_item(text, data));
      items.push_back({text, data});
    }
  };
  
  rows_added(logsModel, 1, 0, logsModel->rowCount()-1, d->type_filters);
  rows_added(logsModel, 2, 0, logsModel->rowCount()-1, d->sender_filters);
  
  connect(logsModel, &QAbstractItemModel::rowsInserted, [=](const QModelIndex & parent, int first, int last){
    rows_added(logsModel, 1, first, last, d->type_filters);
    rows_added(logsModel, 2, first, last, d->sender_filters);
  });
  
  void (QComboBox:: *activatedSignal)(int) = &QComboBox::activated;
  connect(d->ui->type, activatedSignal, [=](int row) { d->filterByType.setFilterFixedString( d->type_filters.item(row)->data().toString() ); });
  connect(d->ui->sender, activatedSignal, [=](int row) { d->filterBySender.setFilterFixedString( d->sender_filters.item(row)->data().toString() ); });
}
