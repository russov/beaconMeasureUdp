#include "mapcontrol.h"
#include "ui_mapcontrol.h"

#include <QtGui/QPainter>

#include <QtWidgets/QMenu>
#include <QtWidgets/QComboBox>

#define coordinateX0 129
#define coordinateY0 177

#define metrToPixel 114


MapControl::MapControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapControl)
{
    ui->setupUi(this);

   bd = new BdData(this);

    connect (ui->imageLab, SIGNAL(customContextMenuRequested(QPoint)), SLOT(onContextMenu(QPoint)));

    connect (ui->testPixmap, SIGNAL(clicked()), SLOT(onMenuPixmap()));

    connect (ui->fillPushButton, SIGNAL(clicked()), SLOT(fillExtraPoints()));
    connect (ui->clearPushButton, SIGNAL(clicked()), SLOT(clearExtraPoints()));

    //beaconData = new BeaconData();
}

MapControl::~MapControl()
{
    delete bd;
    bd = NULL;

   // delete beaconData;

    delete ui;
}

void MapControl::setXD(const std::vector<std::vector<double> >& v)
{
    xd = v;
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

void MapControl::onCurrentIndexChanged(const QString &text)
{
    const QPixmap* map = ui->imageLab->pixmap();
    QSize size = ui->imageLab->size();

    double gy = (double)lastPoint.ry() / size.height();
    double y = map->height() * gy;

    double gx = (double)lastPoint.rx() / size.width();
    double x = map->width() * gx;

    double xMetr = double(x - coordinateX0) / metrToPixel;
    double yMetr = double(y - coordinateY0) / metrToPixel;


    QStringList textList = text.split(" ");

    QString uuid = textList[0];
    QString major = textList[1];
    QString minor= textList[2];
    QString name = textList[3];

    bd->updateCoordinateBeacon(BdData::NameBeacon(uuid, major, minor, name), xMetr, yMetr);

    /*
    if (beaconPoints.isEmpty())
        beaconPoints.insert(1, QPair <int, int> (x,y) );
    else
        beaconPoints.insert(beaconPoints.lastKey() + 1, QPair <int, int> (x,y));
    */


    drawPoints();


    ((QComboBox *)sender())->close();
    ((QComboBox *)sender())->deleteLater();
}

void MapControl::drawPoints()
{
    ui->imageLab->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->imageLab->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    ui->imageLab->setScaledContents(true);

    QImage image("f:\\Projects\\Beacone\\ImprovedTriangulation\\Plan2.bmp");

    QPainter painter;
    painter.begin(&image);
    painter.setPen( Qt::green );
    painter.setBrush( Qt::green );

    foreach (const BdData::NameCoordinatesBeacon& var, bd->getBeaconsCoordinatesName())
        painter.drawEllipse((var.x * metrToPixel) + coordinateX0 - 25
                            , (var.y * metrToPixel) + coordinateY0 - 25, 50, 50);

    painter.end();

    ui->imageLab->setPixmap(QPixmap::fromImage(image));
}

void MapControl::drawPoints(const std::vector<std::vector<double> > &Points, const std::vector<double> &size, const std::vector<Qt::GlobalColor> color)
{
    drawPoints();

    QPainter painter;
    QImage image(ui->imageLab->pixmap()->toImage());

    painter.begin(&image);
    for (int i = 0; i < 200; ++i)
    {
        painter.setPen( color[i] );
        painter.drawEllipse(Points[i][0] * metrToPixel  + coordinateX0 - size[i]/2
                , Points[i][1] * metrToPixel + coordinateY0 - size[i]/2, size[i], size[i]);
    }
    painter.end();

    ui->imageLab->setPixmap(QPixmap::fromImage(image));
}

void MapControl::drawPoint(const std::vector<double> &Point, double size,
                Qt::GlobalColor Color)
{
    QPainter painter;
    QImage image(ui->imageLab->pixmap()->toImage());

    painter.begin(&image);
    painter.setPen( Color );
    painter.setBrush(Color );
    painter.drawEllipse(Point[0] * metrToPixel  + coordinateX0-size/2
            , Point[1] * metrToPixel + coordinateY0 - size/2, size, size);
    painter.end();

    ui->imageLab->setPixmap(QPixmap::fromImage(image));

}



void MapControl::clear()
{
    drawPoints();
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
    QComboBox *combobox = new QComboBox();

    QList <BdData::NameBeacon> names = bd->getBeacons();

    combobox->addItem("1");

    foreach (const BdData::NameBeacon & name, names)
        combobox->addItem(name.uuid + " " + name.major + " " + name.minor + " " + name.name);

    connect(combobox, SIGNAL(currentTextChanged(QString)), SLOT(onCurrentIndexChanged(QString)));

    combobox->show();
}

void MapControl::onMenuPixmap()
{
    setFileName();
}
