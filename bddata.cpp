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
        beacon.x = query.value(4).toDouble();
        beacon.y = query.value(5).toDouble();

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

QMap< QString, int > BdData::getBeaconsId(int position)
{
    static QMap< QString, int > beacons;

    if (beacons.empty())
    {
        QSqlQuery query(ConnectDB::getInstance()->getDBase());

        QString request(QString("select id, uuid, major, minor, name from beacons "));

        //qDebug() << request;

        query.exec(request);

        while (query.next())
        {
            beacons.insert(query.value(1).toString()
                                        + query.value(2).toString()
                                        + query.value(3).toString()
                                        + query.value(4).toString()
                                        , query.value(0).toInt());
        }
    }
    return beacons;
}

int BdData::getCountValue(int beaconId, int position)
{
    int count = 0;
    static QMap <QPair <int, int>, int > cacheCountValue;

    if (cacheCountValue.find(qMakePair(beaconId, position)) == cacheCountValue.end() )
    {
        QSqlQuery query(ConnectDB::getInstance()->getDBase());

        QString request(QString("select sum(count) from histogram "
                                "where id_beacon = %1 and id_position = "
                                "(select id from positions where num = %2)")
                        .arg(beaconId)
                        .arg(position));

        //qDebug() << request;

        query.exec(request);

        while (query.next())
        {
            count = query.value(0).toInt();
        }

        cacheCountValue.insert(qMakePair(beaconId, position), count);
    }
    return cacheCountValue.value(qMakePair(beaconId, position));
}

QMap<int, int> BdData::getHistogram(int beaconId, int position)
{
    QMap <int, int> histogram;
    static QMap <QPair <int, int>, QMap <int, int> > cacheHistogram;

    if (cacheHistogram.find(qMakePair(beaconId, position)) == cacheHistogram.end() )
    {
        QSqlQuery query(ConnectDB::getInstance()->getDBase());

        QString request(QString("select rssi, count from histogram "
                                "where id_beacon = %1 and id_position "
                                "= (select id from positions where num = %2)")
                        .arg(beaconId)
                        .arg(position));

        //qDebug() << request;

        query.exec(request);

        while (query.next())
            histogram.insert(query.value(0).toInt(), query.value(1).toInt());

        cacheHistogram.insert(qMakePair(beaconId, position), histogram);
    }
    return cacheHistogram.value(qMakePair(beaconId, position));
}

QPair<double, double> BdData::getCoordinatesPoint(int position)
{
    QPair <double, double> coordinate;
    static QMap <int, QPair <double, double> > cacheCoordinate;

    if (cacheCoordinate.find(position) == cacheCoordinate.end())
    {
        QSqlQuery query(ConnectDB::getInstance()->getDBase());

        QString request(QString("select x, y from positions "
                                "where num = %1")
                        .arg(position));

        query.exec(request);

        while (query.next())
        {
            coordinate = qMakePair (query.value(0).toDouble(), query.value(1).toDouble());
        }
        cacheCoordinate.insert(position, coordinate);
    }
    return cacheCoordinate.value(position);
}

QList < QString > BdData::getPocketBeaconData(int position)
{
    QList < QString > pocketBeaconData;

    QSqlQuery query(ConnectDB::getInstance()->getDBase());

    QString request(QString("select time_measure, uuid, major, minor, tx_power, "
                            "rssi, sequence_number_pocket, sequence_number_position,"
                            "name "
                            "from databeacon "
                            "where sequence_number_position = %1")
                    .arg(position));

    query.exec(request);

    qDebug() << request;

    while (query.next())
    {
        pocketBeaconData.push_back(
                    query.value(0).toString() + ";"
                    + query.value(1).toString() + ";"
                    + query.value(2).toString() + ";"
                    + query.value(3).toString() + ";"
                    + query.value(4).toString() + ";"
                    + query.value(5).toString() + ";"
                    + query.value(6).toString() + ";"
                    + query.value(7).toString() + ";"
                    + query.value(8).toString() + ";"
                    + "0");
    }
    return pocketBeaconData;
}


void BdData::updateCoordinateBeacon(const BdData::NameBeacon &name, double x, double y)
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
