#include <QtCore/QDebug>

#include <QDateTime>

#include "beacondata.h"
#include "parsebeacondata.h"

#include "gpf.h"

QMap<QString, int> beaconRssiAverage;

struct Beacon
{
    int x;
    int y;
    double P0;
    double n;
};

QMap<QString, Beacon> beaconInitData;

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

    xd.resize(200, std::vector<double>(2));\

    std::default_random_engine generator1, generator2;
    std::uniform_real_distribution<double> distribution1(0.0, 1.0);
    std::uniform_real_distribution<double> distribution2(0.0, 1.0);

    std::vector<double> size;
    std::vector<Qt::GlobalColor> color;

    for (int i=0; i<200; ++i)
    {
       double number = distribution1(generator1);
       xd[i][0] = 127 + (1475-127)*number;

       number = distribution2(generator1);
       xd[i][1] = 173 + (1293-173)*number;

       wt.push_back(1/200.0);

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


    ////////

    BdData *data = new BdData();

    QList<BdData::NameCoordinatesBeacon> nameCoordinatesBeacon = data->getBeaconsCoordinatesName();



    foreach (const BdData::NameCoordinatesBeacon &arg, nameCoordinatesBeacon)
    {
        Beacon beacon;
        beacon.x = arg.x;
        beacon.y = arg.y;
        beacon.P0 = data->getTxPowerBeacon(arg.uuid, arg.major, arg.minor, arg.name);
        beacon.n = 2;

        beaconInitData.insert(arg.uuid + arg.major + arg.minor + arg.name, beacon);
    }
    delete data;
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
    beaconRssiAverage = calculateAverageRssi();

    std::vector<double> Step;
    Step.push_back(0);
    Step.push_back(0);


    ParticleFilter->particleFilterUpdate(process,
                                         observation,
                                         likelihood,
                                         Step,
                                         Step,
                                         round(200*0.60)
                                        );
    for(unsigned short i = 0; i < 200; ++i)
    {

        wt[i] = ParticleFilter->getParticleState(xd[i], xd[i], i)*500;
    }
    map->clear();
    map->drawPoints(xd, wt);

}


//------------

void process(std::vector<double> &xk, const std::vector<double> &xkm1, const std::vector<double> &step, void* data)
{
    static double StepInaccuracy = 10;  //Check
    std::default_random_engine *generator = (std::default_random_engine *)data;
    std::normal_distribution<double> distribution(0.0, StepInaccuracy);


    for (unsigned short i=0; i<xk.size(); i++)
    {
        xk[i] = xkm1[i] + step[i] + distribution(*generator);
    }
}


void observation(std::vector<double> &zk, const std::vector<double> &xk, void* data)
{
    double Likelihood = 0;
    static const double Sigma_RSS = 7;
    static const double Scale = 0.00867952522255192878338278931751;

//    for(unsigned short i = 0; i<beaconRssiAverage.size(); ++i)
//    {
//        if(beaconRssiAverage.)
//    }

    foreach (const QString &name, beaconRssiAverage.keys())
    {

        double RSS = beaconRssiAverage[name];

        if( (fabs(RSS) > 0) && (beaconInitData[name].x != 0))
        {
            double RSS_0 = beaconInitData[name].P0;
            double n = beaconInitData[name].n;
            double d = sqrt(pow(xk[0] - beaconInitData[name].x,2) + pow(xk[1] - beaconInitData[name].y,2))*Scale;

            Likelihood += (exp(- pow((RSS - (RSS_0 - 10*n*log(d) ) ),2)/(2*Sigma_RSS*Sigma_RSS))/fabs(RSS));

        }
    }
    zk[0] = Likelihood;

}

double likelihood(const std::vector<double> &z, const std::vector<double> &zhat, void* data)
{
  return zhat[0];
}
