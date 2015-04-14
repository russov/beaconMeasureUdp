#include <QtCore/QDebug>

#include <QDateTime>

#include "beacondata.h"
#include "parsebeacondata.h"

#include "gpf.h"

BeaconData::BeaconData(QObject *parent)
    : QObject(parent), udpSocket(NULL)
{
    initSocket();

    //bd = new BdData(this);
    map = new MapControl();
    map->show();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(onProcessTimer()));
    timer->start(500);

    ParticleFilter = new PF::pf(200, 2, 1, PF::WRSWR);

    std::vector <double> wt;
    std::vector < std::vector<double> > xd;

    xd.resize(200, std::vector<double>(2));\

    std::default_random_engine generator1, generator2;
    std::uniform_real_distribution<double> distribution1(0.0, 1.0);
    std::uniform_real_distribution<double> distribution2(0.0, 1.0);

    std::vector<int> size;
    std::vector<Qt::GlobalColor> color;

    for (int i=0; i<200; ++i)
    {
       double number = distribution1(generator1);
       xd[i][0] = 127 + (1475-127)*number;

       number = distribution1(generator1);
       xd[i][1] = 173 + (1293-173)*number;

       wt.push_back(1/200);

       //!!!!
       size.push_back(30);
       color.push_back(Qt::red);
       //!!!!!
    }
//    Constant	Value	Description
//    Qt::white	3	White (#ffffff)
//    Qt::black	2	Black (#000000)
//    Qt::red	7	Red (#ff0000)
//    Qt::darkRed	13	Dark red (#800000)
//    Qt::green	8	Green (#00ff00)
//    Qt::darkGreen	14	Dark green (#008000)
//    Qt::blue	9	Blue (#0000ff)
//    Qt::darkBlue	15	Dark blue (#000080)
//    Qt::cyan	10	Cyan (#00ffff)
//    Qt::darkCyan	16	Dark cyan (#008080)
//    Qt::magenta	11	Magenta (#ff00ff)
//    Qt::darkMagenta	17	Dark magenta (#800080)
//    Qt::yellow	12	Yellow (#ffff00)
//    Qt::darkYellow	18	Dark yellow (#808000)
//    Qt::gray	5	Gray (#a0a0a4)
//    Qt::darkGray	4	Dark gray (#808080)
//    Qt::lightGray	6	Light gray (#c0c0c0)
//    Qt::transparent	19	a transparent black value (i.e., QColor(0, 0, 0, 0))
//    Qt::color0	0	0 pixel value (for bitmaps)
//    Qt::color1	1	1 pixel value (for bitmaps)

    // map->setXD(xd);
    // map->drawPoints();

    map->drawPoints(xd, size, color);

    //map->clear();

    ParticleFilter->initialize(1, wt, xd);
}

BeaconData::~BeaconData()
{
    //delete bd;
   // bd = NULL;

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


    //ParticleFilter->



}
