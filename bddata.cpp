#include "bddata.h"

#include <QtCore/QDebug>
#include <QtCore/QRegExp>
#include <QtCore/QDateTime>

#include <QtSql/QSqlQuery>


BdData::BdData(QObject *parent) : QObject(parent)
{
    dbase = QSqlDatabase::addDatabase("QSQLITE");
    dbase.setDatabaseName("../BeaconData");

    if (!dbase.open())
    {
        qDebug() << "BAD";
    }
/*
    QString request = "CREATE TABLE `DataBeacon` ("
            "`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
            "`time_measure`	INTEGER,"
            "`uuid`	TEXT,"
            "`major`	INTEGER,"
            "`minor`	INTEGER,"
            "`tx_power`	INTEGER,"
            "`channel`	INTEGER,"
            "`rssi`	INTEGER,"
            "`sequence_number_pocket`	INTEGER,"
            "`sequence_number_position`	INTEGER,"
            "`name_id`	INTEGER,"
            "`name`	TEXT,"
            "`time_create`	TEXT,"
            "FOREIGN KEY(`name_id`) REFERENCES Beacon ( id )"
        ")";

    QSqlQuery query(dbase);
    query.exec(request);


    /*QSqlQuery query(dbase);

    if (!query.exec("select * from beacon"))
    {
        qDebug() << "bad select";

    }*/
}

BdData::~BdData()
{
    dbase.close();
}

QList<BdData::NameBeacon> BdData::getBeacons()
{
    QList < BdData::NameBeacon > names;

    QSqlQuery query(dbase);
    QString request("select uuid, major, minor, name from tempbeacon ");

    qDebug() << request;

    query.exec(request);

    while (query.next())
    {
        BdData::NameBeacon beacon;
        beacon.uuid = query.value(0).toString();
        beacon.major = query.value(1).toString();
        beacon.minor = query.value(2).toString();
        beacon.name = query.value(3).toString();

        names.append(beacon);
    }
    return names;
}

QList<BdData::NameCoordinatesBeacon> BdData::getBeaconsCoordinatesName()
{
    QList < BdData::NameCoordinatesBeacon > names;

    QSqlQuery query(dbase);
    QString request("select uuid, major, minor, name, x, y from tempbeacon ");

    qDebug() << request;

    query.exec(request);

    while (query.next())
    {
        BdData::NameCoordinatesBeacon beacon;
        beacon.uuid = query.value(0).toString();
        beacon.major = query.value(1).toString();
        beacon.minor = query.value(2).toString();
        beacon.name = query.value(3).toString();
        beacon.x = query.value(4).toInt();
        beacon.y = query.value(5).toInt();

        names.append(beacon);
    }
    return names;
}

void BdData::updateCoordinateBeacon(const BdData::NameBeacon &name, int x, int y)
{
    QSqlQuery queryUpdate(dbase);
    QString requestUpdate(QString("update TempBeacon "
                                  "set X = %1, Y = %2 "
                                  "where uuid = '%3' and major = '%4' and minor = '%5' and name = '%6' "
                                  "" )
                          .arg(x)
                          .arg(y)
                          .arg(name.uuid)
                          .arg(name.major)
                          .arg(name.minor)
                          .arg(name.name));

    qDebug() << requestUpdate;

    queryUpdate.exec(requestUpdate);
}
