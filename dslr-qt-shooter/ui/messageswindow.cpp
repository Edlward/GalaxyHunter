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

class MessagesWindow::Private {
public:
  Private(Ui::MessagesWindow* ui, QAbstractItemModel* logsModel, MessagesWindow* q);
  std::unique_ptr<Ui::MessagesWindow> ui;
  QAbstractItemModel *logsModel;
private:
  MessagesWindow *q;
};

MessagesWindow::Private::Private(Ui::MessagesWindow* ui, QAbstractItemModel *logsModel, MessagesWindow* q)
  : q(q), logsModel(logsModel), ui(ui)
{

}


MessagesWindow::~MessagesWindow()
{
}

MessagesWindow::MessagesWindow(QAbstractItemModel *logsModel, QWidget* parent) : QWidget(parent), d(new Private{new Ui::MessagesWindow, logsModel, this})
{
  d->ui->setupUi(this);
  d->ui->logs->setModel(logsModel);
}
