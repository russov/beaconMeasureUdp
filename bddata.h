#ifndef BDDATA_H
#define BDDATA_H

#include <QObject>
#include <QtSql/QSqlDatabase>

#include <QtCore/QStringList>
#include <QtCore/QTimer>

class ConnectDB : public QObject
{
    Q_OBJECT
private:
    static ConnectDB * p_instance;
    QSqlDatabase dbase;
    // Конструкторы и оператор присваивания недоступны клиентам
    ConnectDB()
    {
        dbase = QSqlDatabase::addDatabase("QSQLITE");
        dbase.setDatabaseName("../BeaconData");

        if (!dbase.open())
        {

        }
    }

    ConnectDB( const ConnectDB& );
    ConnectDB& operator=( ConnectDB& );

public:
    static ConnectDB * getInstance()
    {
        static ConnectDB *p_instance = new ConnectDB();
        return p_instance;
    }

    QSqlDatabase getDBase()
    {
        return dbase;
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

    QMap<QString, QVector<int> > getRssiPosition(int position);
    QMap<QString, QVector<int> > getRssiPosition(int position, const QString uuid, const QString major,
                                                 const QString minor, const QString name);

    void updateCoordinateBeacon(const NameBeacon &name, int x, int y);
    int getTxPowerBeacon(const QString& uuid, const QString& major, const QString& minor, const QString& name);

protected:

signals:

public slots:

};

#endif // BDDATA_H
