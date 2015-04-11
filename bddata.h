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

protected:
    QSqlDatabase dbase;

signals:

public slots:

};

#endif // BDDATA_H
