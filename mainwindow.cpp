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
    QApplication::quit();
}


void MainWindow::on_m_pBConnect_clicked()
{
    if(ui->m_pBConnect->text() == QString("Start"))
    {
        if (ui->checkMode->isChecked())
        {
            ui->m_textStatus->clear();
            gattServer = GattServer::getInstance();
            if(gattServer)
            {
                QObject::connect(gattServer, &GattServer::connectionState, this, &MainWindow::onConnectionStatedChanged);
                QObject::connect(gattServer, &GattServer::dataReceived, this, &MainWindow::onDataReceived);
                QObject::connect(gattServer, &GattServer::sendInfo, this, &MainWindow::onInfoReceived);
                QString textToAppend = "SERVICEUUID   6E400001-B5A3-F393-E0A9-E50E24DCCA9E\n"
                                       "RXUUID        6E400002-B5A3-F393-E0A9-E50E24DCCA9E\n"
                                       "TXUUID        6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

                appendText(textToAppend);
                gattServer->startBleService();
                gatt_started = true;
            }
            ui->m_pBConnect->setText("Stop");
        }

    }
    else
    {
        if (ui->checkMode->isChecked())
        {
            gattServer->stopBluetoothService();
            gatt_started = false;
        }

        ui->m_pBConnect->setText("Start");
    }
}


void MainWindow::on_checkMode_stateChanged(int arg1)
{

    if (arg1 != 0)
    {
        ui->m_textStatus->clear();
        gattServer = GattServer::getInstance();
        if(gattServer)
        {
            QObject::connect(gattServer, &GattServer::connectionState, this, &MainWindow::onConnectionStatedChanged);
            QObject::connect(gattServer, &GattServer::dataReceived, this, &MainWindow::onDataReceived);
            QObject::connect(gattServer, &GattServer::sendInfo, this, &MainWindow::onInfoReceived);
            QString textToAppend = "SERVICEUUID   6E400001-B5A3-F393-E0A9-E50E24DCCA9E\n"
                                   "RXUUID        6E400002-B5A3-F393-E0A9-E50E24DCCA9E\n"
                                   "TXUUID        6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

            appendText(textToAppend);
            gattServer->startBleService();
            gatt_started = true;
            ui->m_pBConnect->setText("Stop");
        }
    }
    else
    {
        if(gattServer)
        {
            gattServer->stopBluetoothService();
            gatt_started = false;
            ui->m_pBConnect->setText("Start");
        }
    }
}

