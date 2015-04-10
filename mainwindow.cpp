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

    connect(ui->everagePushButton, SIGNAL(clicked()), SLOT(onProcessEverageButton()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onProcessEverageButton()
{
    data->calculateEverage();
}
