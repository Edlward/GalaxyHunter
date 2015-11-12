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
#include "ui_addsequenceitem.h"
#include "camerasetup.h"
#include "dslr_shooter_window.h"
#include <QDialog>
#include "Qt/strings.h"
#include <QLabel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QComboBox>
#include <Qt/functional.h>
#include <Qt/qlambdaevent.h>

using namespace std;

class SequenceItem {
public:
  typedef shared_ptr<SequenceItem> Ptr;
  SequenceItem(const SequenceElement& sequence, QStandardItemModel* model);
  ~SequenceItem();
  SequenceElement sequenceElement;
  QStandardItemModel *model;
  operator QList<QStandardItem*>() const;
  QList<shared_ptr<QStandardItem>> columns;
  
  Ptr duplicate() const;
  
};

SequenceItem::Ptr SequenceItem::duplicate() const
{
  return {};
}


SequenceItem::SequenceItem(const SequenceElement& sequenceElement, QStandardItemModel* model) : sequenceElement(sequenceElement), model{model}
{
  QString shots = "";
  QString exposure = "";
  if(sequenceElement.imagingSequence) {
    auto sequence = sequenceElement.imagingSequence;
    shots = "1";
    if(sequence->settings().mode == ShooterSettings::Continuous)
      shots = qApp->tr("infinite");
    if(sequence->settings().mode == ShooterSettings::Sequence)
      shots = QString::number(sequence->settings().shots);
    exposure = sequence->imagerSettings().manualExposure ? 
      QTime{0,0,0}.addSecs(sequence->imagerSettings().manualExposureSeconds).toString() :
      sequence->imagerSettings().shutterSpeed.current + qApp->tr(" (preset)");
    qDebug() << "shots: " << shots << ", exposure: " << exposure;
  }
  columns.push_back(make_shared<QStandardItem>(sequenceElement.displayName));
  columns.push_back(make_shared<QStandardItem>(shots));
  columns.push_back(make_shared<QStandardItem>(exposure));
}

SequenceItem::~SequenceItem()
{
  auto row = model->indexFromItem(columns[0].get()).row();
  qDebug() << "removing row " << row;
  columns.clear();
  model->removeRow(row);
}

QDebug operator<<(QDebug dbg, const SequenceItem::Ptr &item) {
  dbg.nospace() << "{" << item->sequenceElement.displayName << ", " << item->sequenceElement.imagingSequence->settings().shots << "}";
  return dbg.space();
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
  void edit_sequence_element(const SequenceElement &sequence_element);
  void clearSequence();
  QStandardItemModel model;
  QList<SequenceItem::Ptr> sequences;
  QAction* add_action;
  QAction* remove_action;
  QAction* move_up_action;
  QAction* move_down_action;
  QAction* clear_action;
  QAction* edit_action;
  QAction* copy_action;
  void enable_selection_actions();
  void waitDialog(const QString& name, bool timeout_enabled, int timeout_seconds, bool& dialog_finished);
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

void SequencesWidget::Private::enable_selection_actions()
{
      auto has_selection = ui->sequenceItems->selectionModel()->hasSelection();
      for(auto action: QList<QAction*>{remove_action, move_down_action, move_up_action, edit_action, copy_action})
	action->setEnabled(has_selection);
}


SequencesWidget::SequencesWidget(ShooterSettings &shooterSettings, QWidget* parent) : QWidget{parent}, dptr(shooterSettings, this)
{
    d->ui->setupUi(this);
    d->ui->toolbarContainer->setLayout(new QVBoxLayout);
    auto toolbar = new QToolBar("sequence toolbar", d->ui->toolbarContainer);
    d->ui->toolbarContainer->layout()->addWidget(toolbar);
    d->add_action = toolbar->addAction(QIcon::fromTheme("list-add"), "New sequence");
    d->remove_action = toolbar->addAction(QIcon::fromTheme("list-remove"), "Remove sequence");
    d->edit_action = toolbar->addAction(QIcon::fromTheme("edit-rename"), "Edit sequence");
    d->copy_action = toolbar->addAction(QIcon::fromTheme("edit-copy"), "Copy sequence");
    d->move_up_action = toolbar->addAction(QIcon::fromTheme("arrow-up"), "Move up");
    d->move_down_action = toolbar->addAction(QIcon::fromTheme("arrow-down"), "Move down");
    d->clear_action = toolbar->addAction(QIcon::fromTheme("edit-clear-list"), "Clear all");
    d->ui->sequenceItems->setModel(&d->model);
    connect(d->add_action, &QAction::triggered, bind(&Private::addSequenceItem, d.get()));
    connect(d->clear_action, &QAction::triggered, bind(&Private::clearSequence, d.get()));
    
    connect(d->clear_action, &QAction::triggered, bind(&QAction::setEnabled, d->remove_action, false));
    d->ui->sequenceItems->setLayout(new QVBoxLayout);
    setImager({});
    d->model.setHorizontalHeaderLabels({tr("Name"), tr("Shots"), tr("Exposure")});
    connect(d->ui->sequenceItems->selectionModel(), &QItemSelectionModel::selectionChanged, bind(&Private::enable_selection_actions, d.get()));
    connect(d->remove_action, &QAction::triggered, [=]{
      if(!d->ui->sequenceItems->selectionModel()->hasSelection())
	return;
      d->sequences.erase(remove_if(begin(d->sequences), end(d->sequences), [=](const SequenceItem::Ptr &i){ return d->ui->sequenceItems->selectionModel()->isSelected(i->columns[0]->index()); }), d->sequences.end());
      d->ui->sequenceItems->selectionModel()->clearSelection();
      d->enable_selection_actions();
    });
    connect(d->copy_action, &QAction::triggered, [=]{
      if(!d->ui->sequenceItems->selectionModel()->hasSelection())
	return;
      auto sequence = *find_if(begin(d->sequences), end(d->sequences), [=](const SequenceItem::Ptr &i){ return d->ui->sequenceItems->selectionModel()->isSelected(i->columns[0]->index()); });
      auto copy = sequence->duplicate();
      d->sequences.push_back(copy);
      d->model.appendRow(*copy);

    });
    connect(d->edit_action, &QAction::triggered, [=]{
      if(!d->ui->sequenceItems->selectionModel()->hasSelection())
	return;
      auto sequence = *find_if(begin(d->sequences), end(d->sequences), [=](const SequenceItem::Ptr &i){ return d->ui->sequenceItems->selectionModel()->isSelected(i->columns[0]->index()); });
      d->edit_sequence_element(sequence->sequenceElement);
    });
    auto move_item = [=](int how){
      if(!d->ui->sequenceItems->selectionModel()->hasSelection())
	return;
      auto row_index = d->ui->sequenceItems->selectionModel()->selectedRows().first().row();
      auto next_index = row_index + how;
      if(next_index < 0 || next_index >= d->model.rowCount())
	return;
      d->sequences.move(row_index, next_index);
      auto row_items = d->model.takeRow(row_index);
      d->model.insertRow(next_index, row_items);
      d->ui->sequenceItems->selectionModel()->select(d->model.index(next_index, 0), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    };
    connect(d->move_up_action, &QAction::triggered, bind(move_item, -1));
    connect(d->move_down_action, &QAction::triggered, bind(move_item, +1));
    
}

Sequence SequencesWidget::sequence() const
{
  Sequence result;
  for(auto element: d->sequences)
    result.enqueue(element->sequenceElement);
  return result;
}

void SequencesWidget::Private::edit_sequence_element(const SequenceElement& sequence_element)
{
  QDialog *dialog = new QDialog(q);
  auto dialog_ui = new Ui::AddSequenceItemDialog;
  dialog_ui->setupUi(dialog);
  dialog->resize(500, 450);
  dialog->setModal(true);
  dialog_ui->item_name->setText(sequence_element.displayName);
  auto cameraSetup = new CameraSetup(shooterSettings);
  cameraSetup->setCamera(imager);
  cameraSetup->values_from(sequence_element.imagingSequence);
  connect(dialog_ui->item_type, F_PTR(QComboBox, currentIndexChanged, int), dialog_ui->item_settings_stack, &QStackedWidget::setCurrentIndex);
  dialog_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  connect(dialog_ui->item_name, &QLineEdit::textChanged, [=](const QString &newtext) { dialog_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!newtext.isEmpty()); });
  dialog_ui->item_settings_stack->insertWidget(0, cameraSetup);
  dialog_ui->item_settings_stack->setCurrentIndex(0);
  if(dialog->exec() != QDialog::Accepted)
    return;
  qDebug() << "sequence with name: " << dialog_ui->item_name->text() << ", settings: " <<  cameraSetup->imagingSequence()->imagerSettings();
  SequenceItem::Ptr sequenceItem;
  if(dialog_ui->item_type->currentIndex() == 0) {
    qDebug() <<__PRETTY_FUNCTION__ << ": thread_id: " << QThread::currentThreadId();
    sequenceItem = make_shared<SequenceItem>(SequenceElement{cameraSetup->imagingSequence(), dialog_ui->item_name->text()}, &model);
  } else if(dialog_ui->item_type->currentIndex() == 1) {
    auto timeout_enabled = dialog_ui->auto_accept->isChecked();
    auto timeout_seconds = QTime{0,0,0}.secsTo(dialog_ui->auto_accept_timeout->time());
    auto name = dialog_ui->item_name->text();
    sequenceItem = make_shared<SequenceItem>(SequenceElement{{}, dialog_ui->item_name->text(), [=]{
      qDebug() << __PRETTY_FUNCTION__;
      bool dialog_finished = false;
      QLambdaEvent *event = new QLambdaEvent([=,&dialog_finished]{
        waitDialog(name, timeout_enabled, timeout_seconds, dialog_finished);
      });
      qApp->postEvent(DSLR_Shooter_Window::instance(), event);
      while(!dialog_finished);
    }}, &model);
  };
  delete dialog_ui;
  delete dialog;
  sequences.push_back(sequenceItem);
  model.appendRow(*sequenceItem);
}


void SequencesWidget::Private::addSequenceItem()
{
  edit_sequence_element({});
}


void SequencesWidget::Private::waitDialog(const QString &name, bool timeout_enabled, int timeout_seconds, bool& dialog_finished)
{
  qDebug() << __PRETTY_FUNCTION__;
    QDialog *waitDialog = new QDialog;
    waitDialog->setLayout(new QVBoxLayout);
    waitDialog->layout()->addWidget(new QLabel("%1: waiting..."_q % name));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    waitDialog->layout()->addWidget(buttonBox);
    if(timeout_enabled && timeout_seconds>0) {
      QTimer *killTimer = new QTimer(waitDialog);
      connect(killTimer, &QTimer::timeout, waitDialog, &QDialog::accept);
      killTimer->start(timeout_seconds*1000);
      QTimer *updateTimer = new QTimer(waitDialog);
      connect(updateTimer, &QTimer::timeout, waitDialog, [=]{ 
        buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok (%1)") % QTime(0,0,0).addMSecs(killTimer->remainingTime()).toString("HH:mm:ss"));
      });
      updateTimer->start(1000);
    };
    connect(buttonBox, &QDialogButtonBox::accepted, waitDialog, &QDialog::accept);
    connect(waitDialog, &QDialog::accepted, waitDialog, &QDialog::deleteLater);
    waitDialog->show();
    connect(waitDialog, &QDialog::finished, [&dialog_finished]{ dialog_finished = true; });
}


void SequencesWidget::setImager(const ImagerPtr& imager)
{
  d->imager = imager;
  d->clearSequence();
  d->enable_selection_actions();
  d->add_action->setEnabled(imager.operator bool());

  d->clear_action->setEnabled(imager.operator bool());
}

void SequencesWidget::Private::clearSequence()
{
  sequences.clear();
}

#include "sequenceswidget.moc"


