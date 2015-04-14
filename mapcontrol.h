#ifndef MAPCONTROL_H
#define MAPCONTROL_H

#include <QWidget>
#include <QtCore/QMap>
#include <QtCore/QPair>

#include "bddata.h"
//#include "beacondata.h"

namespace Ui {
class MapControl;
}

class MapControl : public QWidget
{
    Q_OBJECT

public:
    explicit MapControl(QWidget *parent = 0);
    ~MapControl();

    void setXD(const std::vector<std::vector<double> > &v);

    void drawPoints();

    void drawPoints(const std::vector<std::vector<double> > &Points, const std::vector<int> &size, const std::vector<Qt::GlobalColor> color);
    void clear();

private:
    Ui::MapControl *ui;

    typedef QPair < int, int > PointPair;

    QMap <int, PointPair > beaconPoints;

    QList <PointPair> extraPoints;

    std::vector<std::vector<double> > xd;

    void setFileName();



    QPoint lastPoint;

    BdData *bd;
  //  BeaconData *beaconData;

public slots:
    void onContextMenu(const QPoint &point);
    void onAddBeacon();
    void onMenuPixmap();

    void fillExtraPoints();
    void clearExtraPoints();

    void onCurrentIndexChanged(const QString& text);

};

#endif // MAPCONTROL_H
