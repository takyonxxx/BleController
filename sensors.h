#ifndef SENSORS_H
#define SENSORS_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QAccelerometer>
#include <QGyroscope>
#include <QPressureSensor>
#include <QDateTime>
//#include "kalmanfilter.h"

#define KF_VAR_ACCEL 0.0075 // Variance of pressure acceleration noise input.
#define KF_VAR_MEASUREMENT 0.05 // Variance of pressure measurement noise.
#define SEA_LEVEL_PRESSURE 101325.0 // Pressure at sea level (Pa)
#define RADIANS_TO_DEGREES 57.2957795

#ifdef Q_OS_ANDROID
    #include <QSensor>
#endif

class Sensors : public QObject
{
    Q_OBJECT
public:
    explicit Sensors(QObject *parent = nullptr);
    static Sensors* getInstance();
public:
    void startSensors();
    void stopSensors();
private:
    QAccelerometer *sensorAcc;
    QAccelerometerReading *accelerometerReading;
    static Sensors *theInstance_;

signals:
    void sendInfo(QString);
    void sendSensorValue(QString);

private slots:
    void accelerometer_changed();
};

#endif // SENSORS_H
