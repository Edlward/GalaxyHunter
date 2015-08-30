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

#include "ledindicator.h"
#include <QBoxLayout>
#include <QLabel>
#include <QLayoutItem>

LedIndicator::~LedIndicator()
{
}

LedIndicator::LedIndicator(LedIndicator::Color color, const QString& text, QWidget* parent)
{
  QBoxLayout *layout = new QHBoxLayout;
  setLayout(layout);
  layout->addWidget(image = new QLabel);
  layout->addWidget(this->text = new QLabel);
  layout->addSpacerItem(new QSpacerItem{0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum});
  setColor(color);
  setText(text);
}

void LedIndicator::setColor(LedIndicator::Color color)
{
  static std::map<Color, QString> images {
    { Red, ":telescope/led-red" },
    { Blue, ":telescope/led-blue" },
    { Green, ":telescope/led-green" },
    { Yellow, ":telescope/led-yellow" },
  };
  this->image->setPixmap(QPixmap::fromImage(QImage(images[color])));
}

void LedIndicator::setText(const QString& text)
{
  this->text->setHidden(text.isEmpty());
  this->text->setText(text);
}

#include "ledindicator.moc"
