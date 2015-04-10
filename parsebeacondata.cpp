#include "parsebeacondata.h"

#include <QtCore/QRegExp>
#include <QtCore/QMap>
#include <QtCore/QDateTime>

ParseBeaconData::ParseBeaconData()
{

}

ParseBeaconData::~ParseBeaconData()
{

}

QMap <QString, QString> ParseBeaconData::ParseData(const QByteArray &array)
{
    QRegExp exp("^([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+);([\\w*\\W*]+)$");

    exp.exactMatch(array);

    QMap <QString, QString> data;
    data.insert("time_measure", exp.cap(1));
    data.insert("uuid", exp.cap(2));
    data.insert("major", exp.cap(3));
    data.insert("minor", exp.cap(4));
    data.insert("tx_power", exp.cap(5));
    data.insert("rssi", exp.cap(6));
    data.insert("sequence_number_pocket", exp.cap(7));
    data.insert("sequence_number_position", exp.cap(8));
    data.insert("name", exp.cap(9));
    data.insert("time_create", QDateTime::currentDateTime().toString("dd-MM-yy HH-mm-ss-zzz"));

    return data;
}

