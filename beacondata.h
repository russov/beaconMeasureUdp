#ifndef BEACONDATA_H
#define BEACONDATA_H

#include <QObject>

#include <QtNetwork/QUdpSocket>

#include <QtCore/QQueue>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>

#include "mapcontrol.h"

#include <bddata.h>
#include "gpf.h"


class BeaconData : public QObject
{
    Q_OBJECT
public:
    explicit BeaconData(QObject *parent = 0);
    ~BeaconData();

    void calculateEverage();

protected:
    void initSocket();
    void processTheDatagram(const QByteArray &datagram);
    QMap <QString, int> calculateAverageRssi();
    QString getUniqNameBeacon(const QMap <QString, QString>& partsBeaconData);

    void deleteBeaconData(int capasityMs = 2000);

    QUdpSocket *udpSocket;
    MapControl *map;

    //BdData *bd;
    QTimer *timer;

    PF::pf *ParticleFilter;

    struct TimeAndDataBeacon
    {
        TimeAndDataBeacon()
        {}
        TimeAndDataBeacon(QDateTime t, const QMap <QString, QString>& d)
            : time(t), dataBeacon(d)
        {}
        QDateTime time;
        QMap <QString, QString> dataBeacon;
    };

    QMap <QString, QList <TimeAndDataBeacon> > actualDataBeacons;

signals:

public slots:
    void readPendingDatagrams();
    void onProcessTimer();
};

#endif // BEACONDATA_H
