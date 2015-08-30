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
#include "Qt/strings.h"
#include <QLabel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
using namespace std;

class SequenceItem {
public:
  SequenceItem(const QString &name, const ImagingSequence::ptr &sequence);
  operator QList<QStandardItem*>() const;
  ImagingSequence::ptr sequence;
  QString name;
  QList<shared_ptr<QStandardItem>> columns;
};

SequenceItem::SequenceItem(const QString& name, const ImagingSequence::ptr& sequence) : sequence{sequence}
{
  QString shots = "1";
  if(sequence->settings().mode == ShooterSettings::Continuous)
    shots = qApp->tr("infinite");
  if(sequence->settings().mode == ShooterSettings::Sequence)
    shots = QString::number(sequence->settings().shots);
  auto exposure = sequence->imagerSettings().manualExposure ? 
    QTime{0,0,0}.addSecs(sequence->imagerSettings().manualExposureSeconds).toString() :
    sequence->imagerSettings().shutterSpeed.current + qApp->tr(" (preset)");
  qDebug() << "shots: " << shots << ", exposure: " << exposure;
  columns.push_back(make_shared<QStandardItem>(name));
  columns.push_back(make_shared<QStandardItem>(shots));
  columns.push_back(make_shared<QStandardItem>(exposure));
}

 SequenceItem::operator QList<QStandardItem*>() const
{
  QList<QStandardItem*> _columns;
  transform(begin(columns), end(columns), back_inserter(_columns), [=](const shared_ptr<QStandardItem> &item) {return item.get(); });
  return _columns;
}


class SequencesWidget::Private {
public:
  Private(ShooterSettings& shooterSettings, SequencesWidget* q);
  ImagerPtr imager;
  ShooterSettings &shooterSettings;
  shared_ptr<Ui::SequencesWidget> ui;
  void addSequenceItem();
  void clearSequence();
  QStandardItemModel model;
  QList<shared_ptr<SequenceItem>> sequences;
    QAction* add_action;
    QAction* remove_action;
    QAction* move_up_action;
    QAction* move_down_action;
    QAction* clear_action;
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
    d->ui->toolbarContainer->setLayout(new QVBoxLayout);
    auto toolbar = new QToolBar("sequence toolbar", d->ui->toolbarContainer);
    d->ui->toolbarContainer->layout()->addWidget(toolbar);
    d->add_action = toolbar->addAction(QIcon::fromTheme("list-add"), "New sequence");
    d->remove_action = toolbar->addAction(QIcon::fromTheme("list-remove"), "Remove sequence");
    d->move_up_action = toolbar->addAction(QIcon::fromTheme("arrow-up"), "Move up");
    d->move_down_action = toolbar->addAction(QIcon::fromTheme("arrow-down"), "Move down");
    d->clear_action = toolbar->addAction(QIcon::fromTheme("edit-clear-list"), "Clear all");
    connect(d->add_action, &QAction::triggered, bind(&Private::addSequenceItem, d.get()));
    connect(d->clear_action, &QAction::triggered, bind(&Private::clearSequence, d.get()));
    d->ui->sequenceItems->setLayout(new QVBoxLayout);
    setImager({});
    d->model.setHorizontalHeaderLabels({tr("Name"), tr("Shots"), tr("Exposure")});
    d->ui->sequenceItems->setModel(&d->model);
    
}

Sequence SequencesWidget::sequence() const
{
}

void SequencesWidget::Private::addSequenceItem()
{
  QDialog *dialog = new QDialog(q);
  dialog->resize(500, 450);
  dialog->setModal(true);
  dialog->setLayout(new QVBoxLayout);
  auto cameraSetup = new CameraSetup(shooterSettings);
  cameraSetup->setCamera(imager);
  dialog->layout()->addWidget(new QLabel{tr("Sequence Name")});
  auto lineedit = new QLineEdit;
  dialog->layout()->addWidget(lineedit);
  dialog->layout()->addWidget(cameraSetup);
  auto buttonBox = new QDialogButtonBox;
  connect(buttonBox->addButton(QDialogButtonBox::Cancel), &QPushButton::clicked, dialog, &QDialog::reject);
  connect(buttonBox->addButton(QDialogButtonBox::Ok), &QPushButton::clicked, dialog, &QDialog::accept);
  dialog->layout()->addWidget(buttonBox);
  if(dialog->exec() != QDialog::Accepted)
    return;
  qDebug() << "sequence with name: " << lineedit->text() << ", settings: " <<  cameraSetup->imagingSequence()->imagerSettings();
  auto sequenceItem = make_shared<SequenceItem>(lineedit->text(), cameraSetup->imagingSequence());
  sequences.push_back(sequenceItem);
  model.appendRow(*sequenceItem);
}

void SequencesWidget::setImager(const ImagerPtr& imager)
{
  d->imager = imager;
  d->clearSequence();
  d->add_action->setEnabled(imager.operator bool());
  d->move_down_action->setDisabled(true);
  d->move_up_action->setDisabled(true);
  d->remove_action->setDisabled(true);
  d->clear_action->setEnabled(imager.operator bool());
}

void SequencesWidget::Private::clearSequence()
{
  sequences.clear();
}

#include "sequenceswidget.moc"


