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

#ifndef LOGMESSAGE_H
#define LOGMESSAGE_H
#include <QMetaType>
#include <QDebug>
#include <QDateTime>

struct LogMessage
{
  enum Type {
    Info = 0,
    Warning = 1,
    Error = 2,
  };
  Type type;
  QString source;
  QString message;
  QDateTime when;
  QString typeDesc() const;
  static LogMessage info(const QString &source, const QString &message);
  static LogMessage warning(const QString &source, const QString &message);
  static LogMessage error(const QString &source, const QString &message);
};


QDebug &operator<<(QDebug &o, const LogMessage &m);




namespace {
  struct ______RegisterLogMessageMetatype {
    ______RegisterLogMessageMetatype() { qRegisterMetaType<LogMessage>("LogMessage"); }
  };
  static ______RegisterLogMessageMetatype ______registerLogMessageMetatype;
};
#endif // LOGMESSAGE_H
