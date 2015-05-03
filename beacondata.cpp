#include <QtCore/QDebug>

#include <QDateTime>

#include "beacondata.h"
#include "parsebeacondata.h"

#include "gpf.h"

#include "simulator.h"

QMap<QString, int> beaconRssiAverage;

struct Beacon
{
    int x;
    int y;
    double P0;
    double n;
};

QMap<QString, Beacon> beaconInitData;

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

    timerTest = new QTimer(this);
    connect(timerTest, SIGNAL(timeout()), SLOT(onProcessTimerTest()));
   // timerTest->start(2000);

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
       xd[i][0] = 11.74 * number;

       number = distribution2(generator1);
       xd[i][1] = 9.42 * number;

       wt.push_back(1/200.0);

       //!!!!
       size.push_back(30);
       color.push_back(Qt::red);
       //!!!!!
    }

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

QPair<double, double> BeaconData::kMeans(QMap<double, int> likelyhoodResult, int capasity)
{
    BdData *data = new BdData();
    double numeratorX = 0;
    double numeratorY = 0;
    double denumerator = 0;

    if (likelyhoodResult.count() != 1 )
        for (QMap<double, int>::iterator iter = likelyhoodResult.end() - 1
             ; iter != likelyhoodResult.end() - capasity - 1
             ; --iter)
        {
            QPair<double, double> coord = data->getCoordinatesPoint(iter.value());

            numeratorX += iter.key() * coord.first;
            numeratorY += iter.key() * coord.second;

            denumerator += iter.key();
        }
    else
    {
        delete data;
        return QPair<double, double> ();
    }
    delete data;

    return qMakePair (numeratorX/denumerator, numeratorY/denumerator);
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

      //  qDebug() << QString(datagram);

      //  qDebug() << countPocket;
        ++countPocket;

        processTheDatagram(datagram);
    }
}

void BeaconData::onProcessTimer()
{
    deleteBeaconData(2000);
    //beaconRssiAverage = calculateAverageRssi();

/*
    Simulator *simulator = new Simulator();

    QMap < QString, std::pair<int, int> > coordinatesBeacon;

    foreach (const QString& name, beaconInitData.keys())
    {
        coordinatesBeacon.insert(name, std::make_pair(beaconInitData.value(name).x, beaconInitData.value(name).y));
        beaconRssiAverage.insert(name,0);
    }


  /*  simulator->generateSimulatorData(beaconRssiAverage
                                     , coordinatesBeacon
                                     , std::make_pair(184,257));
*/
   /* bool i = 0;
    foreach (const QString& name, beaconInitData.keys())
    {
        int value = beaconRssiAverage.take(name);
        if (name.contains("Kontakt"))
        {
            if (i == 0)
            {
                beaconRssiAverage.insert(name, -69);
                i = true;
            }
            else
                beaconRssiAverage.insert(name, -80);
        }

    }
*/
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

    std::vector <double> Position;
    Position.resize(2);
    ParticleFilter->filterOutput(Position);
    map->drawPoint(Position, 30, Qt::darkGreen);


    /*QMap <double, int> likelyhoodResult;


    for (int i = 1; i <= 24; ++i)
    {
        likelyhoodResult.insert(calculateHypothesis(i), i);


    }

  //  foreach (double arg, likelyhoodResult.keys())
    //{
        double arg = likelyhoodResult.lastKey();

        qDebug() << QString::number(arg) << " -- " << QString::number(likelyhoodResult.value(arg));
    //}





        std::vector <double> coord;

        BdData *data = new BdData();
        QPair <double, double> coordinatePoint = kMeans(likelyhoodResult);//data->getCoordinatesPoint(likelyhoodResult.value(arg));
        delete data;

        coord.push_back(coordinatePoint.first);
        coord.push_back(coordinatePoint.second);
        map->clear();
        map->drawPoint(coord, 50, Qt::blue);
*/



}

double BeaconData::calculateHypothesis(int position)
{
    BdData *data = new BdData();

    double summ = 0;

    foreach (const QString& nameBeacon, actualDataBeacons.keys())
    {
        if (actualDataBeacons.value(nameBeacon).isEmpty())
            continue;

        int beaconId = (data->getBeaconsId(position)).value(nameBeacon);
        QMap <int, int> histogram = data->getHistogram(beaconId, position);
        int countValue = data->getCountValue(beaconId, position);

        //TimeAndDataBeacon beacon;
        foreach (const TimeAndDataBeacon &arg, actualDataBeacons.value(nameBeacon))
        {
            summ += (double)histogram.value(arg.dataBeacon.value("rssi").toInt()) / countValue;
        }
    }

   //qDebug() << QString::number(summ);

    delete data;
    return summ;
}

void BeaconData::onProcessTimerTest()
{
    BdData *data = new BdData();

    QByteArray utf8;

    foreach (const QString & arg, data->getPocketBeaconData(1))
    {
        utf8.append(arg);
        processTheDatagram (utf8);
        utf8.clear();
    }
    delete data;

timerTest->stop();


  /*  time_measure, uuid, major, minor, tx_power, "
                                "rssi, sequence_number_pocket, sequence_number_position,"
                                "name "

    */


//    QUdpSocket *udpSocketSend = new QUdpSocket(this);

//                    QHostAddress *bcast = new QHostAddress("127.0.0.1");

//                    udpSocketSend->connectToHost(*bcast,12345);

//                    QByteArray *datagram = QByteArray(strData); // data from external function
//                    udpSocketSend->write(*datagram);

//    delete data;
}


//------------

void process(std::vector<double> &xk, const std::vector<double> &xkm1, const std::vector<double> &step, void* data)
{
    static double StepInaccuracy = 0.5;  //Check
    std::default_random_engine *generator = (std::default_random_engine *)data;
    std::normal_distribution<double> distribution(0.0, StepInaccuracy);


    for (unsigned short i=0; i<xk.size(); i++)
    {
        xk[i] = xkm1[i] + step[i] + distribution(*generator);
    }
}


//void observation(std::vector<double> &zk, const std::vector<double> &xk, void* data)
//{
//    double Likelihood = 0.00000000001;
//    //double LikelihoodTest = 0;
//    static const double Sigma_RSS = 7;
//    static const double Scale = 0.00867952522255192878338278931751;

////    for(unsigned short i = 0; i<beaconRssiAverage.size(); ++i)
////    {
////        if(beaconRssiAverage.)
////    }

//    foreach (const QString &name, beaconRssiAverage.keys())
//    {

//        double RSS = beaconRssiAverage[name];

//        if( (fabs(RSS) > 0) && (beaconInitData[name].x != 0))
//        {
//            double RSS_0 = beaconInitData[name].P0;
//            //double RSS_0 = -59;

//            double n = 1.5;//beaconInitData[name].n;
//            double d = sqrt(pow(xk[0] - beaconInitData[name].x,2) + pow(xk[1] - beaconInitData[name].y,2))*Scale;
//            //double dd = sqrt(pow(184 - beaconInitData[name].x,2) + pow(257 - beaconInitData[name].y,2))*Scale;


//            //double l= log(dd);
//            //double TT = RSS - (RSS_0 - 10*n*l );

//            Likelihood += (exp(- pow((RSS - (RSS_0 - 10*n*log10(d) ) ),2)/(2*Sigma_RSS*Sigma_RSS))/1/*fabs(RSS)*/);
//            //LikelihoodTest += (exp(- pow((RSS - (RSS_0 - 10*n*l ) ),2)/(2*Sigma_RSS*Sigma_RSS))/1/*fabs(RSS)*/);


//        }
//    }
//    zk[0] = Likelihood;

//}


void observation(std::vector<double> &zk, const std::vector<double> &xk, void* data)
{
    Q_UNUSED(data);

    double Likelihood = 0.00000000001;

    static double delta = 0.5;

    BdData *data1 = new BdData();

    for (int i = 1; i <= 3; ++i)
    {
        if ((fabs(xk[0] - data1->getCoordinatesPoint(i).first) < delta)
                && (fabs(xk[1] - data1->getCoordinatesPoint(i).second) < delta))
        {
            Likelihood = BeaconData::calculateHypothesis(i);
            break;
        }
    }


    delete data1;

    zk[0] = Likelihood;

}

double likelihood(const std::vector<double> &z, const std::vector<double> &zhat, void* data)
{
  return zhat[0];
}
