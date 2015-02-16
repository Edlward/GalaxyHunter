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

#ifndef INDICLIENT_H
#define INDICLIENT_H

#include <QObject>
#include <memory>
#include <vector>
#include <indibase.h>

class LogMessage;
class INDIClient : public QObject
{
    Q_OBJECT

public:
    ~INDIClient();
    INDIClient(QObject* parent = 0);
    std::vector< INDI::BaseDevice* > devices() const;
    void sendNewSwitch(ISwitchVectorProperty *s);
    void sendNewText(ITextVectorProperty* _property);
    void sendNewNumber(INumberVectorProperty* _property);
    std::vector<INDI::BaseDevice*> telescopes() const;
public slots:
  void open(const QString &address, int port);
signals:
  void devicesUpdated();
  void newSwitch(ISwitchVectorProperty*);
  void newText(ITextVectorProperty*);
  void newNumber(INumberVectorProperty*);
  void newLight(ILightVectorProperty*);
  void propertyRemoved(INDI::Property*);
  void propertyAdded(INDI::Property*);
  void message(const LogMessage &);
private:
  class Private;
  friend class Private;
  std::unique_ptr<Private> d;
};

#endif // INDICLIENT_H
