#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class BeaconData;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    BeaconData *data;

private slots:

};

#endif // MAINWINDOW_H
