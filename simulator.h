#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QWidget>
#include <QMap>

namespace Ui {
class Simulator;
}

class Simulator : public QWidget
{
    Q_OBJECT

public:
    explicit Simulator(QWidget *parent = 0);
    ~Simulator();

    void generateSimulatorData(QMap <QString, int> &beaconRssiAverage
                               , const QMap < QString, std::pair<int, int> >& coordinatesBeacon
                               , const std::pair <int, int>& coordinatesPoints);

private:
    Ui::Simulator *ui;
};

#endif // SIMULATOR_H
