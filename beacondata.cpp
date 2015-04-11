#include <QtCore/QDebug>

#include <QDateTime>

#include "beacondata.h"
#include "parsebeacondata.h"

#include "gpf.h"

BeaconData::BeaconData(QObject *parent)
    : QObject(parent), udpSocket(NULL)
{
    initSocket();

    bd = new BdData(this);
    map = new MapControl();
    map->show();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(onProcessTimer()));
    timer->start(500);
}

BeaconData::~BeaconData()
{
    delete bd;
    bd = NULL;

    delete map;
    map = NULL;
}

void BeaconData::initSocket()
{
    udpSocket = new QUdpSocket(this);

    udpSocket->bind(12345);

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
}

void BeaconData::processTheDatagram(const QByteArray &datagram)
{
    QMap <QString, QString> partsBeaconData = ParseBeaconData::ParseData(datagram);

    if (actualDataBeacons.find(getUniqNameBeacon(partsBeaconData)) == actualDataBeacons.end())
    {
        QList < TimeAndDataBeacon > startListBeaconData;
        startListBeaconData.push_back(TimeAndDataBeacon(QDateTime::currentDateTime(), partsBeaconData));
        actualDataBeacons.insert(getUniqNameBeacon(partsBeaconData)
                , startListBeaconData);
    } else
    {
        actualDataBeacons[getUniqNameBeacon(partsBeaconData)]
                .push_back(TimeAndDataBeacon(QDateTime::currentDateTime()
                        /*QDateTime::fromString(partsBeaconData["time_create"]
                        , "dd-MM-yy HH-mm-ss-zzz")*/, partsBeaconData));
    }
}

QMap<QString, int> BeaconData::calculateAverageRssi()
{
    QMap<QString, int> beaconRssiAverage;

    foreach (const QString& key, actualDataBeacons.keys())
    {
        int sumRssi = 0;
        foreach (const TimeAndDataBeacon& rssi, actualDataBeacons.value(key))
        {
            sumRssi += rssi.dataBeacon.value("rssi").toInt();
        }

        if (sumRssi)
            beaconRssiAverage.insert(key, sumRssi/actualDataBeacons.value(key).count());
        else
            beaconRssiAverage.insert(key, 0);
    }
    return beaconRssiAverage;
}

QString BeaconData::getUniqNameBeacon(const QMap<QString, QString> &partsBeaconData)
{
    return partsBeaconData["uuid"] + partsBeaconData["major"]
            + partsBeaconData["minor"] + partsBeaconData["name"];
}

void BeaconData::deleteBeaconData(int capasityMs)
{
    foreach (const QString& key, actualDataBeacons.keys())
    {
    /*   int y = (actualDataBeacons.value(key).last().time.toMSecsSinceEpoch()
                 - actualDataBeacons.value(key).first().time.toMSecsSinceEpoch());

        int y2 = (QDateTime::currentDateTime().toMSecsSinceEpoch()
                 - actualDataBeacons.value(key).first().time.toMSecsSinceEpoch());
*/
      /*  foreach (const TimeAndDataBeacon &arg, actualDataBeacons.value(key))
        {
            if ((QDateTime::currentDateTime().toMSecsSinceEpoch()
                    - arg.time.toMSecsSinceEpoch()) > capasityMs)
            {
                actualDataBeacons[key].removeFirst();
            }
        }*/
        while(!actualDataBeacons.value(key).isEmpty())
        {
            int f = QDateTime::currentDateTime().toMSecsSinceEpoch()
               - actualDataBeacons.value(key).first().time.toMSecsSinceEpoch();

            if(( f > capasityMs))
                actualDataBeacons[key].removeFirst();
            else
                break;
        }

     /*   while (!actualDataBeacons.value(key).isEmpty()
               && ( (y = (QDateTime::currentDateTime().toMSecsSinceEpoch()
                - actualDataBeacons.value(key).first().time.toMSecsSinceEpoch())) > capasityMs) )
        {
           // actualDataBeacons[key].removeFirst();
        }*/
    }
}

void BeaconData::readPendingDatagrams()
{
    static int countPocket = 1;

    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        qDebug() << QString(datagram);

        qDebug() << countPocket;
        ++countPocket;

        processTheDatagram(datagram);
    }
}

void BeaconData::onProcessTimer()
{
    deleteBeaconData(1000);
    QMap<QString, int> beaconRssiAverage = calculateAverageRssi();
}
