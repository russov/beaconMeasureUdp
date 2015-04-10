#ifndef BDDATA_H
#define BDDATA_H

#include <QObject>
#include <QtSql/QSqlDatabase>

#include <QtCore/QStringList>
#include <QtCore/QTimer>

class BdData : public QObject
{
    Q_OBJECT
public:
    explicit BdData(QObject *parent = 0);
    ~BdData();

    void insertBeaconData(const QString &data, bool saveData = false);
    void calculateEverage();

protected:
    QSqlDatabase dbase;
    QStringList listBeaconData;
    QTimer *timer;

signals:

public slots:
    void onProcessTimer();
};

#endif // BDDATA_H
