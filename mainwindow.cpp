#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("Ble Remote Control"));

    ui->m_textStatus->setStyleSheet("font-size: 12pt; color: #cccccc; background-color: #003333;");
    ui->m_pBConnect->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #336699;");
    ui->m_pBExit->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #239566;");

    gattServer = GattServer::getInstance();
    if (gattServer)
    {
        qDebug() << "Starting gatt service";
        QObject::connect(gattServer, &GattServer::connectionState, this, &MainWindow::onConnectionStatedChanged);
        QObject::connect(gattServer, &GattServer::dataReceived, this, &MainWindow::onDataReceived);
        QObject::connect(gattServer, &GattServer::sendInfo, this, &MainWindow::onInfoReceived);
        gattServer->startBleService();
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::appendText(QString text)
{
    ui->m_textStatus->append(text);
}

void MainWindow::onConnectionStatedChanged(bool state)
{
    if(state)
    {
        appendText("Bluetooth connection is succesfull.");
    }
    else
    {
        appendText("Bluetooth connection lost.");
    }
}

void MainWindow::onDataReceived(QByteArray data)
{

}

void MainWindow::onInfoReceived(QString info)
{
    appendText(info);
}

void MainWindow::on_m_pBExit_clicked()
{
    exit(0);
}


void MainWindow::on_m_pBConnect_clicked()
{
    if(ui->m_pBConnect->text() == QString("Connect"))
    {
        ui->m_pBConnect->setText("Disconnect");
    }
    else
    {
        ui->m_textStatus->clear();
        ui->m_pBConnect->setText("Connect");
    }
}

