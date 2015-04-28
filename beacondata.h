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
#include <math.h>


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

    std::vector <double> wt;
    std::vector < std::vector<double> > xd;

signals:

public slots:
    void readPendingDatagrams();
    void onProcessTimer();
};

void process(std::vector<double> &xk, const std::vector<double> &xkm1, const std::vector<double> &step, void* data);
void observation(std::vector<double> &zk, const std::vector<double> &xk, void* data);
double likelihood(const std::vector<double> &z, const std::vector<double> &zhat, void* data);

#endif // BEACONDATA_H
