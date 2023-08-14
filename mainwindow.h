#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void dataHandler(QByteArray data);    
    void statusChanged(const QString &status);
    void on_m_pBExit_clicked();
    void on_m_pBConnect_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
