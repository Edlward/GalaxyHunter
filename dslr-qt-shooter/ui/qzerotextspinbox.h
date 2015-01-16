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

#ifndef QZEROTEXTSPINBOX_H
#define QZEROTEXTSPINBOX_H

#include <qt/QtWidgets/QSpinBox>

class QZeroTextSpinBox : public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY(QString zeroText READ zeroText WRITE setZeroText)
public:
    ~QZeroTextSpinBox();
    QZeroTextSpinBox(QWidget* parent);
    QString zeroText() const;
    void setZeroText(const QString &zeroText);

protected:
    virtual QValidator::State validate(QString& input, int& pos) const;
    virtual int valueFromText(const QString& text) const;
    virtual QString textFromValue(int val) const;

private:
  QString _zeroText;
  QIntValidator *validator;
};

#endif // QZEROTEXTSPINBOX_H
