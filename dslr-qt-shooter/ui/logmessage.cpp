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

#include "logmessage.h"


QDebug& operator<<(QDebug& o, const LogMessage& m)
{
  o << "[type=" << m.typeDesc() << "; source=" << m.source << "; message=" << m.message << "]";
  return o;
}
LogMessage LogMessage::error(const QString& source, const QString& message)
{
  return {Error, source, message, QDateTime::currentDateTime()};
}

LogMessage LogMessage::info(const QString& source, const QString& message)
{
  return {Info, source, message, QDateTime::currentDateTime()};
}

LogMessage LogMessage::warning(const QString& source, const QString& message)
{
  return {Warning, source, message, QDateTime::currentDateTime()};
}


QString LogMessage::typeDesc() const
{
  static std::map<Type, QString> descs {
    {Error, "Error"},
    {Warning, "Warning"},
    {Info, "Info"},
  };
  return descs[type];
}
