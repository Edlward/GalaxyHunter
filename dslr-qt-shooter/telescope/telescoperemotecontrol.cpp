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

#include "telescoperemotecontrol.h"
#include "indiclient.h"
#include "switchvectorproperty.h"
#include "numbervectorproperty.h"
#include "ui_telescoperemotecontrol.h"
#include <basedevice.h>
#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <cmath>
/* Partially copied from kstars inditelescope.cpp */
/*  INDI CCD
    Copyright (C) 2012 Jasem Mutlaq <mutlaqja@ikarustech.com>
 
    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 */
 

class TelescopeRemoteControl::Private {
public:
  Private(Ui::TelescopeRemoteControl* ui, const std::shared_ptr<INDIClient> &client, INDI::BaseDevice *device, TelescopeRemoteControl *q);
    std::unique_ptr<Ui::TelescopeRemoteControl> ui;
    std::shared_ptr<INDIClient> client;
    INDI::BaseDevice *device;
    QSqlDatabase db;
    QStandardItemModel cataloguesModel;
    QStandardItemModel objectsModel;
    void loadCatalogues();
    void search(const QString& searchString, long long int catalogue);
    enum CatalogueColumn {
      id = Qt::UserRole + 1,
      name = Qt::UserRole + 2,
      code = Qt::UserRole + 3,
      priority = Qt::UserRole + 4,
      search_mode = Qt::UserRole + 5,
      hidden = Qt::UserRole + 6
    };
    static std::map<TelescopeRemoteControl::Private::CatalogueColumn, QString> catalogue_columns;
    void goTo(double ra, double dec);
private:
  TelescopeRemoteControl *q;
};

TelescopeRemoteControl::Private::Private(Ui::TelescopeRemoteControl* ui, const std::shared_ptr< INDIClient >& client, INDI::BaseDevice* device, TelescopeRemoteControl* q)
  : ui(ui), client(client), device(device), db(QSqlDatabase::addDatabase("QSQLITE")), q(q)
{
}

std::map<TelescopeRemoteControl::Private::CatalogueColumn, QString> TelescopeRemoteControl::Private::catalogue_columns{
  { CatalogueColumn::id, "id"},
  { CatalogueColumn::name, "name"},
  { CatalogueColumn::code, "code"},
  { CatalogueColumn::priority, "priority"},
  { CatalogueColumn::search_mode, "search_mode"},
  { CatalogueColumn::hidden, "hidden"},
};

TelescopeRemoteControl::~TelescopeRemoteControl()
{
  qDebug() << "closing database";
  d->db.close();
}

TelescopeRemoteControl::TelescopeRemoteControl(const std::shared_ptr<INDIClient> &client, INDI::BaseDevice *device, QWidget* parent)
  : QDialog(parent), d(new Private{new Ui::TelescopeRemoteControl, client, device, this})
{
    d->db.setDatabaseName(OBJECTS_DATABASE);
    qDebug() << "open database: " << d->db.open();
    d->ui->setupUi(this);
    QVBoxLayout *manualMoveLayout = new QVBoxLayout;
    d->ui->manualMove->setLayout(manualMoveLayout);
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    manualMoveLayout->addLayout(controlsLayout);
    
    controlsLayout->addWidget(new SwitchVectorProperty(device->getProperty("TELESCOPE_MOTION_NS", INDI_SWITCH)->getSwitch(), client));
    controlsLayout->addWidget(new SwitchVectorProperty(device->getProperty("TELESCOPE_MOTION_WE", INDI_SWITCH)->getSwitch(), client));
    controlsLayout->addWidget(new SwitchVectorProperty(device->getProperty("TELESCOPE_ABORT_MOTION", INDI_SWITCH)->getSwitch(), client));
    manualMoveLayout->addWidget(new SwitchVectorProperty(device->getProperty("SLEWMODE", INDI_SWITCH)->getSwitch(), client));
    QVBoxLayout *coordinatesLayout = new QVBoxLayout;
    d->ui->coordinates->setLayout(coordinatesLayout);
    coordinatesLayout->addWidget(new NumberVectorProperty(device->getProperty("EQUATORIAL_EOD_COORD", INDI_NUMBER)->getNumber(), client));
    d->ui->catalogue->setModel(&d->cataloguesModel);
    
    d->ui->objects_results->setModel(&d->objectsModel);
    connect(d->ui->objectName, &QLineEdit::textChanged, [=](const QString &s) { d->ui->searchObject->setEnabled(s.size()>0); });
    auto searchObject = [=]{
      int64_t catalogue = d->cataloguesModel.item(d->ui->catalogue->currentIndex(), 0)->data(Private::id).toLongLong();
      d->search(d->ui->objectName->text(), catalogue);
    };
    connect(d->ui->objectName, &QLineEdit::returnPressed, searchObject);
    connect(d->ui->searchObject, &QPushButton::clicked, searchObject);
    connect(d->ui->objects_results, &QTreeView::activated, [=](const QModelIndex &index) { d->ui->gotoObject->setEnabled(index.isValid()); });
    connect(d->ui->gotoObject, &QPushButton::clicked, [=]{
      auto item = d->objectsModel.itemFromIndex(d->ui->objects_results->currentIndex());
      auto skyObjectId =  item->data().toLongLong();
      QSqlQuery query;
      query.prepare("SELECT ra, dec from objects WHERE id = :object_id");
      query.bindValue(":object_id", skyObjectId);
      if(query.exec() && query.next()) {
	double ra = query.value("ra").toDouble() * 180. / M_PI;
	ra *= (24./360.);
	double dec = query.value("dec").toDouble() * 180. / M_PI;
	d->goTo(ra, dec);
      } else
	qDebug() << "query failed: " << query.executedQuery() << ", error: " << query.lastError();
    });
    d->loadCatalogues();
}

void TelescopeRemoteControl::Private::goTo(double ra, double dec)
{
  qDebug() << __PRETTY_FUNCTION__ << ": ra: " << ra << ", dec=" << dec;
  ISwitchVectorProperty *motion = device->getSwitch("ON_COORD_SET");
  if(!motion) return;
  ISwitch *sw = IUFindSwitch(motion, "TRACK");
  if(!sw)
    sw = IUFindSwitch(motion, "SLEW");
  if(!sw)
    return;
  if(sw->s != ISS_ON) {
    IUResetSwitch(motion);
    sw->s = ISS_ON;
    client->sendNewSwitch(motion);
  }

  INumber *RAEle(NULL), *DecEle(NULL), *AzEle(NULL), *AltEle(NULL);
  INumberVectorProperty *EqProp(NULL), *HorProp(NULL);
  double currentRA=0, currentDEC=0, currentAlt=0, currentAz=0;
  bool useJ2000 (false);

  EqProp = device->getNumber("EQUATORIAL_EOD_COORD");
  if (EqProp == NULL)
  {
  // J2000 Property
      EqProp = device->getNumber("EQUATORIAL_COORD");
      if (EqProp)
	  useJ2000 = true;

  }

  HorProp = device->getNumber("HORIZONTAL_COORD");

  if (EqProp && EqProp->p == IP_RO)
      EqProp = NULL;

  if (HorProp && HorProp->p == IP_RO)
	  HorProp = NULL;

//qDebug() << "Skymap click - RA: " << scope_target->ra().toHMSString() << " DEC: " << scope_target->dec().toDMSString();

    if (EqProp)
    {
	    RAEle  = IUFindNumber(EqProp, "RA");
	    if (!RAEle) return;
	    DecEle = IUFindNumber(EqProp, "DEC");
		if (!DecEle) return;

// 	if (useJ2000)
// 	    ScopeTarget->apparentCoord(KStars::Instance()->data()->ut().djd(), (long double) J2000);
// 
	  currentRA  = RAEle->value;
	  currentDEC = DecEle->value;
	  RAEle->value  = ra;
	  DecEle->value = dec;
    }

//     if (HorProp)
//     {
// 	    AzEle = IUFindNumber(HorProp, "AZ");
// 	    if (!AzEle) return false;
// 	    AltEle = IUFindNumber(HorProp,"ALT");
// 	    if (!AltEle) return false;
// 
// 	currentAz  = AzEle->value;
// 	currentAlt = AltEle->value;
// 	AzEle->value  = ScopeTarget->az().Degrees();
// 	AltEle->value = ScopeTarget->alt().Degrees();
//     }

    /* Could not find either properties! */
    if (EqProp == NULL && HorProp == NULL)
	return;

    if (EqProp)
    {
	client->sendNewNumber(EqProp);
	RAEle->value = currentRA;
	DecEle->value = currentDEC;
    }
//     if (HorProp)
//     {
// 	client->sendNewNumber(HorProp);
// 	AzEle->value  = currentAz;
// 	AltEle->value = currentAlt;
//     }
}


void TelescopeRemoteControl::Private::loadCatalogues()
{
    cataloguesModel.clear();
    QSqlQuery query("SELECT id, name, code, priority, search_mode, hidden from catalogues order by priority ASC");
    qDebug() << "running query: " << query.exec() << ", active: " << query.isActive() << ", select: " << query.isSelect() << ", results: " << query.size() << ", lastError: " << query.lastError();
    while (query.next()) {
      auto item = new QStandardItem(query.value("name").toString());
      for(auto col: catalogue_columns)
	item->setData(query.value(col.second), col.first);
      cataloguesModel.appendRow(item);
    }
}

void TelescopeRemoteControl::Private::search(const QString& searchString, long long catalogue)
{
  objectsModel.clear();
  objectsModel.setHorizontalHeaderLabels({"Catalogue", "Number", "Name", "Comment", "RA", "DEC", "Magnitude", "Size", "Type", "Constellation"});
  qDebug() << "Searching for " << searchString << " in catalogue " << catalogue;
  QSqlQuery query;
  query.prepare(R"(select objects.id, catalogues.name, denominations.number, denominations.name, 
		  denominations.comment, objects.ra, objects.dec, objects.magnitude, objects.angular_size, objects.type, objects.constellation_abbrev 
from denominations inner join catalogues on denominations.catalogues_id = catalogues.id
inner join objects on objects.id = denominations.objects_id WHERE catalogues.id = :catalogue_id AND 
(denominations.name LIKE '%' || :search_str || '%' OR denominations.number LIKE '%' || :search_str || '%' ) )");
  query.bindValue(":catalogue_id", catalogue);
  query.bindValue(":search_str", searchString);
  qDebug() << "query: " << query.exec() << ", results: " << query.size() << ", error: " << query.lastError() << ", executed: " << query.executedQuery();
  QSqlRecord record = query.record();
  while (query.next()) {
    QList<QStandardItem*> items;
    for(int i=1; i<record.count(); i++) {
      qDebug() << record.fieldName(i) << ": " << query.value(i) << " ";
      QStandardItem *item = new QStandardItem(query.value(i).toString());
      item->setData(query.value(0));
      items.push_back(item);
    }
    objectsModel.appendRow(items);
  }
}


