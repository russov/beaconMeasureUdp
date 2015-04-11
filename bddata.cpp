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
