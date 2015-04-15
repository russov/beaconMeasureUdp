#ifndef BDDATA_H
#define BDDATA_H

#include <QObject>
#include <QtSql/QSqlDatabase>

#include <QtCore/QStringList>
#include <QtCore/QTimer>

class ConnectDB : public QObject
{
    Q_OBJECT
public:
    static QSqlDatabase getDBase()
    {
        QSqlDatabase dbase;
        dbase = QSqlDatabase::addDatabase("QSQLITE");
        dbase.setDatabaseName("../BeaconData");

        if (!dbase.open())
        {
            //qDebug() << "BAD";
        }
        return dbase;
    }

    ~ConnectDB()
    {
        ConnectDB::getDBase().close();
    }
};

class BdData : public QObject
{
    Q_OBJECT
public:
    explicit BdData(QObject *parent = 0);
    ~BdData();

    struct NameBeacon
    {
        NameBeacon()
        {}

        NameBeacon(QString u, QString maj, QString min, QString n)
            : uuid(u), major(maj), minor(min), name(n)
        {}

        QString uuid;
        QString major;
        QString minor;
        QString name;
    };

    struct NameCoordinatesBeacon : public NameBeacon
    {
        NameCoordinatesBeacon()
            : NameBeacon()
        {}

        NameCoordinatesBeacon(QString u, QString maj, QString min, QString n, int x1, int y1)
            : NameBeacon(u, maj, min, n), x(x1), y(y1)
        {}

        int x;
        int y;
    };

    QList < NameBeacon > getBeacons();
    QList<NameCoordinatesBeacon> getBeaconsCoordinatesName();

    void updateCoordinateBeacon(const NameBeacon &name, int x, int y);
    int getTxPowerBeacon(const QString& uuid, const QString& major, const QString& minor, const QString& name);

protected:
    //QSqlDatabase dbase;



signals:

public slots:

};

#endif // BDDATA_H
