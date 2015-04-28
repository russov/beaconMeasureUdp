#include "bddata.h"

#include <QtCore/QDebug>
#include <QtCore/QRegExp>
#include <QtCore/QDateTime>

#include <QtSql/QSqlQuery>


BdData::BdData(QObject *parent) : QObject(parent)
{
  /*  dbase = QSqlDatabase::addDatabase("QSQLITE");
    dbase.setDatabaseName("../BeaconData");

    if (!dbase.open())
    {
        qDebug() << "BAD";
    }*/
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
  // dbase.close();
}

QList<BdData::NameBeacon> BdData::getBeacons()
{
    QList < BdData::NameBeacon > names;

    QSqlQuery query(ConnectDB::getInstance()->getDBase());
    QString request("select uuid, major, minor, name from tempbeacon ");

    //qDebug() << request;

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

    QSqlQuery query(ConnectDB::getInstance()->getDBase());
    QString request("select uuid, major, minor, name, x, y from tempbeacon ");

    //qDebug() << request;

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

QMap<QString, QVector<int> > BdData::getRssiPosition(int position)
{
    QMap<QString, QVector<int> > rssiPosition;

    QSqlQuery query(ConnectDB::getInstance()->getDBase());
    QString request(QString("select uuid, major, minor, name, rssi from databeacon "
                    "where sequence_number_position = %1")
                    .arg(position));

    //qDebug() << request;

    query.exec(request);

    while (query.next())
    {
        rssiPosition[query.value(0).toString() + query.value(1).toString()
                + query.value(2).toString() + query.value(3).toString()].push_back(query.value(4).toInt());
    }
    return rssiPosition;
}

QMap<QString, QVector<int> > BdData::getRssiPosition(int position, const QString uuid, const QString major, const QString minor, const QString name)
{
    QMap<QString, QVector<int> > rssiPosition;

    QSqlQuery query(ConnectDB::getInstance()->getDBase());

    QString request(QString("select uuid, major, minor, name, rssi from databeacon "
                            "where sequence_number_position = %1 and uuid = '%2' "
                            "and major = '%3' and minor = '%4' and name = '%5' ")
                    .arg(position)
                    .arg(uuid)
                    .arg(major)
                    .arg(minor)
                    .arg(name));

    //qDebug() << request;

    query.exec(request);

    while (query.next())
    {
        rssiPosition[query.value(0).toString() + query.value(1).toString()
                + query.value(2).toString() + query.value(3).toString()].push_back(query.value(4).toInt());
    }
    return rssiPosition;
}

void BdData::updateCoordinateBeacon(const BdData::NameBeacon &name, int x, int y)
{
    QSqlQuery queryUpdate(ConnectDB::getInstance()->getDBase());
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

    //qDebug() << requestUpdate;

    queryUpdate.exec(requestUpdate);
}

int BdData::getTxPowerBeacon(const QString& uuid, const QString& major, const QString& minor, const QString& name)
{
    int txPower;

    QSqlQuery query(ConnectDB::getInstance()->getDBase());
    QString request(QString("select DISTINCT tx_power from databeacon "
                    "where uuid = '%1' and major = '%2' and minor = '%3' and name = '%4' ")
                    .arg(uuid)
                    .arg(major)
                    .arg(minor)
                    .arg(name));

    //qDebug() << request;

    query.exec(request);

    while (query.next())
    {
        txPower = query.value(0).toInt();
    }
    return txPower;
}
