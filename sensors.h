#ifndef SENSORS_H
#define SENSORS_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QAccelerometer>
#include <QGyroscope>
#include <QPressureSensor>
#include <QDateTime>
#include "kalmanfilter.h"

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
public:
    void getSensorInfo();
private:

    QTimer *writeTimer{};

    QPressureSensor *sensorPress;
    QPressureReading *pressureReading;

    QAccelerometer *sensorAcc;
    QAccelerometerReading *accelerometerReading;

    QGyroscope *sensorGyr;
    QGyroscopeReading * gyroscopeReading;

    QString text_presssure;
    QDateTime p_start;
    QDateTime p_end;
    QDateTime a_start;
    QDateTime a_end;
    QDateTime g_start;
    QDateTime g_end;

    KalmanFilter *pitch_filter;
    KalmanFilter *roll_filter;
    KalmanFilter *yaw_filter;
    KalmanFilter *slipskid_filter;
    KalmanFilter *turnrate_filter;
    KalmanFilter *pressure_filter;
    KalmanFilter *altitude_filter;
    qreal p_dt;
    qreal a_dt;
    qreal g_dt;
    qreal pressure;
    qreal temperature;
    qreal baroaltitude;
    qreal altitude;
    qreal vario;
    qreal offset;
    qreal oldaltitude;

    int tCount;

    qreal alpha             ;
    qreal beta              ;
    qreal roll              ;
    qreal pitch             ;
    qreal yaw               ;
    qreal heading           ;
    qreal slipSkid          ;
    qreal turnRate          ;
    qreal devH              ;
    qreal devV              ;
    qreal groundspeed       ;
    qreal sensoralt         ;
    qreal sensorpressure    ;
    qreal climbRate         ;
    qreal machNo            ;
    qreal adf               ;
    qreal distance          ;

    quint64 lastPressTimestamp ;
    qreal pDeltaT ;

    quint64 lastAccTimestamp ;
    qreal aDeltaT ;

    quint64 lastGyroTimestamp;      ///< Most recent gyroscope measurement timestamp
    qreal gDeltaT;                  ///< Latest time slice for angular velocity

    void writeValuePeriodically();
signals:
    void sendInfo(QString);

private slots:
    void gyroscope_changed();
    void pressure_changed();
    void accelerometer_changed();
};

#endif // SENSORS_H
