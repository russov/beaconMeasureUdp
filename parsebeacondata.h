#ifndef PARSEBEACONDATA_H
#define PARSEBEACONDATA_H

#include <QtCore/QByteArray>
#include <QtCore/QStringList>

class ParseBeaconData
{
public:
    ParseBeaconData();
    ~ParseBeaconData();

    static QMap<QString, QString> ParseData(const QByteArray &array);
};

#endif // PARSEBEACONDATA_H
