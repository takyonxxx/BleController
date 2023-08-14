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

void MainWindow::statusChanged(const QString &status)
{
    ui->m_textStatus->append(status);
}


void MainWindow::dataHandler(QByteArray data)
{

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

