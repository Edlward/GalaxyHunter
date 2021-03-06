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
using namespace std::placeholders;

class SequenceItem {
public:
  typedef shared_ptr<SequenceItem> Ptr;
  SequenceItem(const SequenceElement& sequence, QStandardItemModel* model);
  ~SequenceItem();
  SequenceElement sequenceElement;
  QStandardItemModel *model;
  operator QList<QStandardItem*>() const;
  QList<shared_ptr<QStandardItem>> columns;
  void update(const SequenceElement &element);
  
  Ptr duplicate() const;
  
};

SequenceItem::Ptr SequenceItem::duplicate() const
{
  auto sequenceElementCopy = sequenceElement;
  sequenceElementCopy.displayName = "Copy of %1"_q % sequenceElement.displayName;
  return make_shared<SequenceItem>(sequenceElementCopy, model);
}


SequenceItem::SequenceItem(const SequenceElement& sequenceElement, QStandardItemModel* model) : sequenceElement(sequenceElement), model{model}
{
  columns.push_back(make_shared<QStandardItem>(sequenceElement.displayName));
  columns.push_back(make_shared<QStandardItem>(sequenceElement.imagingSequence ? sequenceElement.imagingSequence->toString() : sequenceElement.wait.toString()));
}

void SequenceItem::update(const SequenceElement& element)
{
  qDebug() << element;
  this->sequenceElement = element;
  columns[0]->setText(element.displayName);
  columns[1]->setText(element.imagingSequence ? element.imagingSequence->toString() : sequenceElement.wait.toString());
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
  SequenceItem::Ptr edit_sequence_element(const SequenceElement& sequence_element);
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
private:
  SequencesWidget *q;
};

SequencesWidget::Private::Private(ShooterSettings& shooterSettings, SequencesWidget* q)
  : shooterSettings(shooterSettings), ui{make_shared<Ui::SequencesWidget>()}, q{q}
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
    d->ui->sequenceItems->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(&d->model, &QAbstractItemModel::dataChanged, [&]{ d->ui->sequenceItems->resizeColumnToContents(1); });
    connect(&d->model, &QAbstractItemModel::rowsInserted, [&]{ d->ui->sequenceItems->resizeColumnToContents(1); });
    connect(d->add_action, &QAction::triggered, bind(&Private::addSequenceItem, d.get()));
    connect(d->clear_action, &QAction::triggered, bind(&Private::clearSequence, d.get()));
    
    connect(d->clear_action, &QAction::triggered, bind(&QAction::setEnabled, d->remove_action, false));
    d->ui->sequenceItems->setLayout(new QVBoxLayout);
    setImager({});
    d->model.setHorizontalHeaderLabels({tr("Name"), tr("Description")});
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
      auto item = d->edit_sequence_element(sequence->sequenceElement);
      if(item) {
	sequence->update(item->sequenceElement);
      }
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

SequenceItem::Ptr SequencesWidget::Private::edit_sequence_element(const SequenceElement& sequence_element)
{
  QDialog *dialog = new QDialog(q);
  auto dialog_ui = new Ui::AddSequenceItemDialog;
  dialog_ui->setupUi(dialog);
  dialog->resize(500, 450);
  dialog->setModal(true);
  dialog_ui->item_name->setText(sequence_element.displayName);

  auto cameraSetup = new CameraSetup(shooterSettings);
  cameraSetup->setCamera(imager, sequence_element.imagingSequence);
  connect(dialog_ui->item_type, F_PTR(QComboBox, currentIndexChanged, int), dialog_ui->item_settings_stack, &QStackedWidget::setCurrentIndex);
  dialog_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!sequence_element.displayName.isEmpty());
  connect(dialog_ui->item_name, &QLineEdit::textChanged, [=](const QString &newtext) { dialog_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!newtext.isEmpty()); });
  connect(dialog_ui->auto_accept, &QCheckBox::toggled, bind(&QLineEdit::setEnabled, dialog_ui->auto_accept_timeout, _1));
  dialog_ui->item_settings_stack->insertWidget(0, cameraSetup);
  dialog_ui->item_settings_stack->setCurrentIndex(0);
  if(! sequence_element.imagingSequence) {
    if(sequence_element.wait) {
      qDebug() << sequence_element.wait.toString();
      dialog_ui->item_settings_stack->setCurrentWidget(dialog_ui->wait_for_page);
      dialog_ui->item_type->setCurrentIndex(1);
      dialog_ui->auto_accept->setChecked(sequence_element.wait.seconds > 0);
      dialog_ui->auto_accept_timeout->setTime(QTime{0,0,0}.addSecs(sequence_element.wait.seconds));
    }
  }
  dialog_ui->auto_accept_timeout->setEnabled(dialog_ui->auto_accept->isChecked());
  if(dialog->exec() != QDialog::Accepted)
    return {};
  auto sequence_element_name = dialog_ui->item_name->text();
  qDebug() << "sequence with name: " << sequence_element_name << ", settings: " <<  cameraSetup->imagingSequence()->imagerSettings();
  SequenceItem::Ptr sequenceItem;
  qDebug() << "current index: " << dialog_ui->item_type->currentIndex();
  if(dialog_ui->item_type->currentIndex() == 0) {
    qDebug() <<__PRETTY_FUNCTION__ << ": thread_id: " << QThread::currentThreadId();
    sequenceItem = make_shared<SequenceItem>(SequenceElement{cameraSetup->imagingSequence(), sequence_element_name}, &model);
  } else if(dialog_ui->item_type->currentIndex() == 1) {
    auto timeout_enabled = dialog_ui->auto_accept->isChecked();
    qint64 timeout_seconds = timeout_enabled ? QTime{0,0,0}.secsTo(dialog_ui->auto_accept_timeout->time()) : 0;
    auto name = sequence_element_name;
    qDebug() << "wait dialog: timeout_enabled=" << timeout_enabled << ", seconds=" << timeout_seconds << ", name=" << sequence_element_name;
    sequenceItem = make_shared<SequenceItem>(SequenceElement{{}, sequence_element_name, {timeout_seconds} }, &model);
  };
  delete dialog_ui;
  delete dialog;
  return sequenceItem;

}


void SequencesWidget::Private::addSequenceItem()
{
  auto item = edit_sequence_element({});
  if(item) {
    sequences.push_back(item);
    model.appendRow(*item);
  }
  
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


