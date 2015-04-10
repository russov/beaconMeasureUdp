#ifndef MAPCONTROL_H
#define MAPCONTROL_H

#include <QWidget>
#include <QtCore/QMap>
#include <QtCore/QPair>

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

public slots:
    void onContextMenu(const QPoint &point);
    void onAddBeacon();
    void onMenuPixmap();

    void fillExtraPoints();
    void clearExtraPoints();
};

#endif // MAPCONTROL_H
