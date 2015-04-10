#include "bddata.h"

#include <QtCore/QDebug>
#include <QtCore/QRegExp>
#include <QtCore/QDateTime>

#include <QtSql/QSqlQuery>


BdData::BdData(QObject *parent) : QObject(parent)
{
    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), SLOT(onProcessTimer()));

    dbase = QSqlDatabase::addDatabase("QSQLITE");
    dbase.setDatabaseName("../BeaconData");

    if (!dbase.open())
    {
        qDebug() << "BAD";
    }

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
  // insertBeaconData(QString(), true);

    dbase.close();

    delete timer;
}

void BdData::insertBeaconData(const QString &data, bool saveData)
{
   /* for (int i = 1; i <= exp.captureCount();++i)
        qDebug() << exp.cap(i);
*/

    if (!data.isEmpty())
        listBeaconData.append(data);

    timer->start(2000);

    if(listBeaconData.count() > 100 || saveData)
    {
        dbase.transaction();
        QSqlQuery query(dbase);

        foreach (const QString &arg, listBeaconData)
        {
            QRegExp exp("^([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+)$");

            exp.exactMatch(arg);

            QString request(QString("INSERT into DataBeacon "
                                    "(time_measure, uuid, major, minor, tx_power, "
                                    "rssi, sequence_number_pocket, sequence_number_position,"
                                    "name, time_create)"
                                    "VALUES (%1, '%2', %3, %4, %5, %6, %7, %8, '%9', '%10') ")
                            .arg(exp.cap(1))
                            .arg(exp.cap(2))
                            .arg(exp.cap(3))
                            .arg(exp.cap(4))
                            .arg(exp.cap(5))
                            .arg(exp.cap(6))
                            .arg(exp.cap(7))
                            .arg(exp.cap(8))
                            .arg(exp.cap(9))
                            .arg(QDateTime::currentDateTime().toString("dd-MM-yy HH-mm-ss-zzz"))
                            );

            if (!query.exec(request))
            {
                qDebug() << "bad insert";
                dbase.rollback();
            }
        }
        query.clear();
        if (!dbase.commit())
            qDebug() << "bad commit";

        listBeaconData.clear();
    }

}

void BdData::calculateEverage()
{
   // QSqlQuery query(dbase);
/*
    QString request = ""
                      "DROP TABLE IF EXISTS TEMPBEACON;"

            "CREATE TABLE `TEMPBEACON` ("
                "`ID`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                "`UUID`	TEXT,"
                "`MAJOR`	INTEGER,"
                "`MINOR`	INTEGER,"
                "`NAME`	TEXT,"
                "`AVERAGE`	INTEGER"
            ");"

            "INSERT INTO TEMPBEACON (UUID, MAJOR, MINOR, NAME) SELECT DISTINCT UUID, MAJOR, MINOR, NAME FROM DATABeacon"
            "ORDER BY UUID, MAJOR, MINOR,NAME;"

            "DROP TABLE IF EXISTS EVERAGERSSI;"
            "CREATE TABLE `EVERAGERSSI` ("
                "`POSITION` INTEGER,"
                "`BEACON_ID` INTEGER,"
                "`RSSI`	INTEGER,"
                "FOREIGN KEY(`BEACON_ID`) REFERENCES TEMPBEACON ( ID )"
            "); ";
*/
//    bool i = query.exec(request);
  //  bool f =0;
    QString request(QString("select `ID`, `UUID`, `MAJOR`, `MINOR`, `NAME` from `TEMPBEACON`"));
    QSqlQuery query(dbase);
    bool k = query.exec(request);

    while (query.next())
    {
        int id = query.value(0).toInt();
        QString uuid = query.value(1).toString();
        int major = query.value(2).toInt();
        int minor = query.value(3).toInt();
        QString name = query.value(4).toString();

        qDebug() << name << uuid;

        QSqlQuery queryPosition(dbase);
        QString requestPosition(QString("select DISTINCT sequence_number_position from databeacon"));
        queryPosition.exec(requestPosition);

        while (queryPosition.next())
        {
            int position = queryPosition.value(0).toInt();

            qDebug() << position;

            QSqlQuery queryEverage(dbase);
            QString requestEverage(QString("select avg(rssi) from databeacon "
                                           "where uuid = '%1' and major = %2 and minor = %3 and name = '%4' and "
                                           "sequence_number_position = %5")
                                   .arg(uuid)
                                   .arg(major)
                                   .arg(minor)
                                   .arg(name)
                                   .arg(position));

            qDebug() << requestEverage;

            queryEverage.exec(requestEverage);

            while (queryEverage.next())
            {
                int rssi = queryEverage.value(0).toInt();

                qDebug() << "average = " << rssi;

                QSqlQuery queryUpdate(dbase);
                QString requestUpdate(QString("insert into EVERAGERSSI (position, beacon_id, rssi) "
                                              "values (%1, %2, %3)" )
                                      .arg(position)
                                      .arg(id)
                                      .arg(rssi));

                qDebug() << requestUpdate;



                queryUpdate.exec(requestUpdate);

            }


        }


    }




}

void BdData::onProcessTimer()
{
    insertBeaconData(QString(), true);

    timer->stop();
}
