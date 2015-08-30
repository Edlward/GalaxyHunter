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

#include "inumberwidget.h"
#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "indidouble.h"
#include <QLayoutItem>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

INumberWidget::~INumberWidget()
{
}

class NumberEditor : public QDoubleSpinBox {
public:
  explicit NumberEditor(const QString &format, QWidget* parent = 0);
  virtual QValidator::State validate(QString& input, int& pos) const;
  virtual QString textFromValue(double val) const;
  virtual double valueFromText(const QString& text) const;
private:
  const QString format;
};

NumberEditor::NumberEditor(const QString& format, QWidget* parent): QDoubleSpinBox(parent), format(format)
{
}

QString NumberEditor::textFromValue(double val) const
{
    return INDIDouble(val, format);
}


QValidator::State NumberEditor::validate(QString& input, int& pos) const
{
    return INDIDouble(input, format).valid() ? QValidator::Acceptable : QValidator::Invalid;
}

double NumberEditor::valueFromText(const QString& text) const
{
    return INDIDouble(text, format);
}


void INumberWidget::setValue(double value)
{
  numberEditor->setValue(value);
}

void INumberWidget::setRange(double min, double max)
{
  numberEditor->setRange(min, max);
}


void INumberWidget::setEnabled(bool enable)
{
  setButton->setVisible(enable);
  numberEditor->setEnabled(enable);
}



INumberWidget::INumberWidget(const QString& label, const QString& format, QWidget* parent): QWidget(parent )
{
  QBoxLayout *layout = new QHBoxLayout;
  setLayout(layout);
  layout->addWidget(new QLabel{label});
  layout->addWidget(numberEditor = new NumberEditor{format}, 1);
  numberEditor->setReadOnly(true);
  numberEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
  if(!isEnabled())
    return;
  setButton = new QPushButton(tr("set"));
  layout->addWidget(setButton);
  connect(setButton, &QPushButton::clicked, [=]{
    QDialog *dialog = new QDialog;
    NumberEditor *editor = new NumberEditor{format};
    editor->setValue(numberEditor->value());
    QBoxLayout *layout = new QVBoxLayout(dialog);
    dialog->setLayout(layout);
    layout->addWidget(editor);
    dialog->setModal(true);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, &QDialogButtonBox::accepted, [=]{
      onValueChanged(editor->value());
    });
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    layout->addWidget(buttonBox);
    dialog->show();
  });
}

void INumberWidget::setOnValueChanged(std::function<void(double)> onValueChanged)
{
  this->onValueChanged = onValueChanged;
}


#include "inumberwidget.moc"
