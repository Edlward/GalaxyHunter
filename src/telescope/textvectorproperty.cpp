/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <email>
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

#include "textvectorproperty.h"
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDialog>

TextVectorProperty::~TextVectorProperty()
{
}

TextVectorProperty::TextVectorProperty(ITextVectorProperty* property, const std::shared_ptr< INDIClient >& indiClient, QWidget* parent)
  : QGroupBox(parent), VectorProperty(property, indiClient, &INDIClient::newText, this)
{
    load(property);
}

int TextVectorProperty::property_size(ITextVectorProperty* property) const
{
  return property->ntp;
}


QWidget* TextVectorProperty::propertyWidget(int index)
{
  IText sw = _property->tp[index];
  QWidget *widget = new QWidget;
  QHBoxLayout *layout = new QHBoxLayout(widget);
  widget->setLayout(layout);
  layout->addWidget(new QLabel(sw.label));
  QLineEdit *lineEdit = new QLineEdit(sw.text);
  lineEdit->setReadOnly(true);
  lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addWidget(lineEdit, 1);
  if(_property->p != IP_RO) {
    QPushButton *change = new QPushButton(tr("set"));
    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    layout->addWidget(change);
    connect(change, &QPushButton::clicked, [=]() {
      QDialog *dialog = new QDialog;
      QLineEdit *editor = new QLineEdit(lineEdit->text());
      QBoxLayout *layout = new QVBoxLayout(dialog);
      dialog->setLayout(layout);
      layout->addWidget(editor);
      dialog->setModal(true);
      auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel);

      connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
      connect(buttonBox, &QDialogButtonBox::accepted, [=]{
        qDebug() << editor->text() << "set: ";
        IUSaveText(&_property->tp[index], editor->text().toLocal8Bit().constData());
        _indiClient->sendNewText(_property);
      });
      connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
      layout->addWidget(buttonBox);
      dialog->show();
    });
  }
  return widget;
}

#include "textvectorproperty.moc"
