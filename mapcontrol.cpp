#include "mapcontrol.h"
#include "ui_mapcontrol.h"

#include <QtGui/QPainter>

#include <QtWidgets/QMenu>

MapControl::MapControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapControl)
{
    ui->setupUi(this);

    connect (ui->imageLab, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onContextMenu(QPoint)));

    connect (ui->testPixmap, SIGNAL(clicked()), SLOT(onMenuPixmap()));

    connect (ui->fillPushButton, SIGNAL(clicked()), SLOT(fillExtraPoints()));
    connect (ui->clearPushButton, SIGNAL(clicked()), SLOT(clearExtraPoints()));
}

MapControl::~MapControl()
{
    delete ui;
}

void MapControl::setFileName()
{
    ui->imageLab->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->imageLab->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    ui->imageLab->setScaledContents(true);

    drawPoints();
}

void MapControl::fillExtraPoints()
{
    extraPoints.append(PointPair(200,300));
    drawPoints();
}

void MapControl::clearExtraPoints()
{
    extraPoints.clear();
    drawPoints();
}

void MapControl::drawPoints()
{
    QImage image("f:\\Projects\\Beacone\\ImprovedTriangulation\\Plan2.bmp");

    QPainter painter;
    painter.begin(&image);
    painter.setPen( Qt::green );
    painter.setBrush( Qt::green );

    foreach(const PointPair& arg, beaconPoints.values())
        painter.drawEllipse(arg.first-25, arg.second-25, 50, 50);

    painter.end();

    painter.begin(&image);
    painter.setPen( Qt::red );
    painter.setBrush( Qt::red );

    foreach(const PointPair& arg, extraPoints)
        painter.drawEllipse(arg.first-15, arg.second-15, 30, 30);

    painter.end();

    ui->imageLab->setPixmap(QPixmap::fromImage(image));
}

void MapControl::onContextMenu(const QPoint& point)
{
    Q_UNUSED(point);

    QMenu menu;
    QAction * actionAddBeacon = new QAction(&menu);
    actionAddBeacon->setText("Add new beacon");
    connect (actionAddBeacon, SIGNAL(triggered()), SLOT(onAddBeacon()));

    menu.addAction(actionAddBeacon);

    lastPoint = point;
    menu.exec(QCursor::pos());

    delete actionAddBeacon;
}

void MapControl::onAddBeacon()
{
    const QPixmap* map = ui->imageLab->pixmap();
    QSize size = ui->imageLab->size();

    double gy = (double)lastPoint.ry() / size.height();
    double y = map->height() * gy;

    double gx = (double)lastPoint.rx() / size.width();
    double x = map->width() * gx;

    if (beaconPoints.isEmpty())
        beaconPoints.insert(1, QPair <int, int> (x,y) );
    else
        beaconPoints.insert(beaconPoints.lastKey() + 1, QPair <int, int> (x,y));

    drawPoints();
}

void MapControl::onMenuPixmap()
{
    setFileName();
}
