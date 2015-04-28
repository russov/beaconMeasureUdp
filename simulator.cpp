#include "simulator.h"
#include "ui_simulator.h"

Simulator::Simulator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Simulator)
{
    ui->setupUi(this);
}

Simulator::~Simulator()
{
    delete ui;
}


void Simulator::generateSimulatorData(QMap <QString, int> &beaconRssiAverage
                           , const QMap < QString, std::pair<int, int> >& coordinatesBeacon
                           , const std::pair <int, int>& coordinatesPoints)
{
    static const double Scale = 0.00867952522255192878338278931751;
    foreach (const QString &name, beaconRssiAverage.keys()) {

        if(coordinatesBeacon[name].first > 1)
        {
            double X = coordinatesBeacon[name].first;
            double Y = coordinatesBeacon[name].second;
            double d = sqrt( pow( coordinatesBeacon[name].first - coordinatesPoints.first,2) + pow( coordinatesBeacon[name].second - coordinatesPoints.second,2) )*Scale;
            beaconRssiAverage[name] = -59 - 10*2*log10( d );
        }
        else
        {
            beaconRssiAverage[name] = 0;
        }

    }
}
