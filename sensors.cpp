#include "sensors.h"

Sensors::Sensors(QObject *parent)
    : QObject{parent}
{
    pressure           = 101325.0;
    altitude           = 0;
    oldaltitude        = 0;
    roll               = 0;
    pitch              = 0;
    yaw                = 0;
    sensorpressure     = 0;
    offset             = 0;
    lastPressTimestamp = 0;
    pDeltaT            = 0;
    lastAccTimestamp   = 0;
    aDeltaT            = 0;
    lastGyroTimestamp  = 0;
    gDeltaT            = 0;
}

void Sensors::getSensorInfo()
{
#ifdef Q_OS_ANDROID
    QList<QByteArray> sensorTypes = QSensor::sensorTypes();
    qDebug() << "Available Sensor Types:";
    QString status;

    foreach (const QByteArray &type, QSensor::sensorTypes()) {
        foreach (const QByteArray &identifier, QSensor::sensorsForType(type)) {
            // Don't put in sensors we can't connect to
            QSensor sensor(type);
            sensor.setIdentifier(identifier);
            if (!sensor.connectToBackend()) {
                qDebug() << "Couldn't connect to " + identifier;
                continue;
            }

            QString sensor_type = type;
            if(sensor_type.contains("Accelerometer"))
            {
                qDebug() << "Type:" << type << "Description:" << sensor.description();
                auto statusText = QString("Sensor %1 %2").arg(type).arg(sensor.description());
                emit sendInfo(statusText);

                sensorAcc = new QAccelerometer(this);
                connect(sensorAcc, SIGNAL(readingChanged()), this, SLOT(accelerometer_changed()));
                sensorAcc->setIdentifier(identifier);

                if (!sensorAcc->connectToBackend()) {
                    qDebug() << "Can't connect to Backend sensor: " + sensor_type;
                    delete sensorAcc;
                    sensorAcc = 0;
                    return;
                }

                if(sensorAcc->start())
                {
                    auto statusText = QString("Sensor %1 %2 started.").arg(type).arg(sensor.description());
                    emit sendInfo(statusText);
                }
            }
            else if(sensor_type.contains("Gyroscope"))
            {
                sensorGyr = new QGyroscope(this);
                connect(sensorGyr, SIGNAL(readingChanged()), this, SLOT(gyroscope_changed()));
                sensorGyr->setIdentifier(identifier);

                if (!sensorGyr->connectToBackend()) {
                    qDebug() << "Can't connect to Backend sensor: " + sensor_type;
                    delete sensorGyr;
                    sensorGyr = 0;
                    return;
                }

                if(sensorGyr->start())
                {
                    g_start = QDateTime::currentDateTime();
                    slipskid_filter = new KalmanFilter(KF_VAR_ACCEL);
                    slipskid_filter->Reset(slipSkid);
                    turnrate_filter = new KalmanFilter(KF_VAR_ACCEL);
                    turnrate_filter->Reset(turnRate);
                    auto statusText = QString("Sensor %1 %2 started.").arg(type).arg(sensor.description());
                    emit sendInfo(statusText);
                }
            }
            else if(sensor_type.contains("Pressure"))
            {
                sensorPress = new QPressureSensor(this);
                connect(sensorPress, SIGNAL(readingChanged()), this, SLOT(pressure_changed()));
                sensorPress->setIdentifier(identifier);

                if (!sensorPress->connectToBackend()) {
                    qDebug() << "Can't connect to Backend sensor: " + sensor_type;
                    delete sensorPress;
                    sensorPress = 0;
                    return;
                }

                if(sensorPress->start())
                {
                    p_start = QDateTime::currentDateTime();
                    pressure_filter = new KalmanFilter(KF_VAR_ACCEL);
                    pressure_filter->Reset(pressure);
                    altitude_filter = new KalmanFilter(KF_VAR_ACCEL);
                    altitude_filter->Reset(altitude);
                    auto statusText = QString("Sensor %1 %2 started.").arg(type).arg(sensor.description());
                    emit sendInfo(statusText);
                }
            }
        }
    }

    writeTimer = new QTimer(this);
    connect(writeTimer, &QTimer::timeout, this, &Sensors::writeValuePeriodically);
    writeTimer->start(1000);
#else
    qDebug() << "This code is meant for Android only.";
    emit sendInfo("This code is meant for Android only.");
#endif
}

void Sensors::writeValuePeriodically()
{

}

void Sensors::gyroscope_changed()
{
    quint64 timestamp = sensorGyr->reading()->timestamp();

    if(lastGyroTimestamp > 0)
    {
        gDeltaT = ((qreal)(timestamp - lastGyroTimestamp))/1000000.0f;
        if(gDeltaT > 0)
        {
            gyroscopeReading = sensorGyr->reading();
            g_end = QDateTime::currentDateTime();
            g_dt = g_start.msecsTo(g_end) / 1000.;

            if(gyroscopeReading != 0 && g_dt > 0)
            {
                qreal x = qDegreesToRadians(gyroscopeReading->property("x").value<qreal>());
                qreal y = qDegreesToRadians(gyroscopeReading->property("y").value<qreal>());
                qreal z = qDegreesToRadians(gyroscopeReading->property("z").value<qreal>());

                slipskid_filter->Update(z, KF_VAR_MEASUREMENT, g_dt);
                slipSkid = slipskid_filter->GetXAbs();

                turnrate_filter->Update(y, KF_VAR_MEASUREMENT, g_dt);
                turnRate = turnrate_filter->GetXAbs();

                g_start = g_end;
            }
        }
    }
    lastGyroTimestamp = timestamp;
}

void Sensors::pressure_changed()
{
    quint64 timestamp = sensorPress->reading()->timestamp();

    if(lastGyroTimestamp > 0)
    {
        pDeltaT = ((qreal)(timestamp - lastPressTimestamp))/1000000.0f;
        if(pDeltaT > 0)
        {
            pressureReading = sensorPress->reading();
            p_end = QDateTime::currentDateTime();
            p_dt = p_start.msecsTo(p_end) / 1000.;

            if(pressureReading != 0 && p_dt > 0)
            {
                pressure = pressureReading->pressure();
                //sensorpressure = pressure * 0.000145037738; //per square inch
                temperature = pressureReading->temperature();
                pressure_filter->Update(pressure, KF_VAR_MEASUREMENT, p_dt);
                pressure = pressure_filter->GetXAbs();
                sensorpressure = pressure * 0.01; //hpa

                baroaltitude = 44330.0 * (1.0 - powf(pressure /SEA_LEVEL_PRESSURE, 0.19));
                altitude_filter->Update(baroaltitude, KF_VAR_MEASUREMENT, p_dt);
                altitude = altitude_filter->GetXAbs();
                sensoralt = altitude;
                vario = altitude_filter->GetXVel();
                climbRate = vario ;

//                if(m_beepThread)
//                    m_beepThread->SetVario(vario, p_dt);

//                fillVario();
//                fillStatus();
                oldaltitude = altitude;
                p_start = p_end;
            }
        }
    }
    lastPressTimestamp = timestamp;
}

void Sensors::accelerometer_changed()
{
    quint64 timestamp = sensorAcc->reading()->timestamp();

    if(lastAccTimestamp > 0)
    {
        aDeltaT = ((qreal)(timestamp - lastAccTimestamp))/1000000.0f;
        if(aDeltaT > 0)
        {
            accelerometerReading = sensorAcc->reading();
            a_end = QDateTime::currentDateTime();
            a_dt = a_start.msecsTo(a_end) / 1000.;

            if(accelerometerReading != 0 && a_dt > 0)
            {
                qreal x = accelerometerReading->property("x").value<qreal>();
                qreal y  =accelerometerReading->property("y").value<qreal>();
                qreal z = accelerometerReading->property("z").value<qreal>();

                pitch = qAtan(y / qSqrt(x * x + z * z)) * RADIANS_TO_DEGREES;
                roll =  qAtan(x / qSqrt(y * y + z * z)) * RADIANS_TO_DEGREES;
                yaw  =  qAtan(z / qSqrt(y * y + x * x)) * RADIANS_TO_DEGREES;

                qreal theta = qAtan(qSqrt(x * x + z * z) / y) * RADIANS_TO_DEGREES;
                if (theta < 0)
                {
                    if (yaw > 0)
                        yaw = 180 - yaw;
                    else
                        yaw = -180 - yaw;
                }

                yaw_filter->Update(yaw,KF_VAR_MEASUREMENT,a_dt);
                yaw = yaw_filter->GetXAbs();

                roll_filter->Update(roll,KF_VAR_MEASUREMENT,a_dt);
                roll = roll_filter->GetXAbs();

                pitch_filter->Update(pitch,KF_VAR_MEASUREMENT,a_dt);
                pitch = pitch_filter->GetXAbs();

                pitch = -1 * yaw;
                roll = -1 * roll;

                a_start = a_end;
            }
        }
    }
    lastAccTimestamp = timestamp;

    auto statusText = QString("pitch: %1 roll: %2 yaw: %3").arg(pitch).arg(roll).arg(yaw);
    emit sendInfo(statusText);
}
