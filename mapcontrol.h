#ifndef MAPCONTROL_H
#define MAPCONTROL_H

#include <QWidget>
#include <QtCore/QMap>
#include <QtCore/QPair>

#include "bddata.h"

namespace Ui {
class MapControl;
}

class MapControl : public QWidget
{
    Q_OBJECT

public:
    explicit MapControl(QWidget *parent = 0);
    ~MapControl();

private:
    Ui::MapControl *ui;

    typedef QPair < int, int > PointPair;

    QMap <int, PointPair > beaconPoints;

    QList <PointPair> extraPoints;

    void setFileName();

    void drawPoints();

    QPoint lastPoint;

    BdData *bd;

public slots:
    void onContextMenu(const QPoint &point);
    void onAddBeacon();
    void onMenuPixmap();

    void fillExtraPoints();
    void clearExtraPoints();

    void onCurrentIndexChanged(const QString& text);
};

#endif // MAPCONTROL_H
