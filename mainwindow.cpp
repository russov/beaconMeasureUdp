#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "beacondata.h"
#include "bddata.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    data = new BeaconData(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
