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

#include "sequenceswidget.h"
#include "ui_sequenceswidget.h"
#include "camerasetup.h"
#include <QDialog>

using namespace std;

class SequencesWidget::Private {
public:
  Private(ShooterSettings& shooterSettings, SequencesWidget* q);
  ImagerPtr imager;
  ShooterSettings &shooterSettings;
  shared_ptr<Ui::SequencesWidget> ui;
  void addSequenceItem();
  void clearSequence();
private:
  SequencesWidget *q;
};

SequencesWidget::Private::Private(ShooterSettings& shooterSettings, SequencesWidget* q)
  : shooterSettings{shooterSettings}, ui{make_shared<Ui::SequencesWidget>()}, q{q}
{

}


SequencesWidget::~SequencesWidget()
{
}

SequencesWidget::SequencesWidget(ShooterSettings &shooterSettings, QWidget* parent) : QWidget{parent}, dptr(shooterSettings, this)
{
    d->ui->setupUi(this);
    connect(d->ui->addSequenceItem, &QPushButton::clicked, bind(&Private::addSequenceItem, d.get()));
    connect(d->ui->clearSequence, &QPushButton::clicked, bind(&Private::clearSequence, d.get()));
    setImager({});
}

Sequence SequencesWidget::sequence() const
{
}

void SequencesWidget::Private::addSequenceItem()
{
  QDialog *dialog = new QDialog(q);
  dialog->setModal(true);
  dialog->setLayout(new QGridLayout);
  auto cameraSetup = new CameraSetup(shooterSettings);
  cameraSetup->setCamera(imager);
  dialog->layout()->addWidget(cameraSetup);
  dialog->exec();
}

void SequencesWidget::setImager(const ImagerPtr& imager)
{
  d->imager = imager;
  d->clearSequence();
  d->ui->addSequenceItem->setEnabled(imager.operator bool());
  d->ui->clearSequence->setEnabled(imager.operator bool());
}

void SequencesWidget::Private::clearSequence()
{
}


