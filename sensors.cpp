#include "sensors.h"

Sensors *Sensors::theInstance_= nullptr;

Sensors* Sensors::getInstance()
{
    if (theInstance_ == nullptr)
    {
        theInstance_ = new Sensors();
    }
    return theInstance_;
}

Sensors::Sensors(QObject *parent)
    : QObject{parent}
{

}

void Sensors::startSensors()
{
#ifdef Q_OS_ANDROID
    QList<QByteArray> sensorTypes = QSensor::sensorTypes();
    qDebug() << "Available Sensor Types:";
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

            auto statusText = QString("Sensor found %1 %2").arg(sensor_type).arg(sensor.description());
            emit sendInfo(statusText);

            if(sensor_type.contains("Accelerometer"))
            {
                if(!sensorAccelerometer)
                {
                    sensorAccelerometer = new QAccelerometer(this);
                    connect(sensorAccelerometer, &QAccelerometer::readingChanged, this, &Sensors::accelerometer_changed);
                    sensorAccelerometer->setIdentifier(identifier);

                    if (!sensorAccelerometer->connectToBackend()) {
                        qDebug() << "Can't connect to Backend sensor: " + sensor_type;
                        auto statusText = QString("Can't connect to Backend sensor: %1").arg(sensor_type);
                        emit sendInfo(statusText);
                        delete sensorAccelerometer;
                        sensorAccelerometer = nullptr;
                        return;
                    }
                }

                if(sensorAccelerometer && sensorAccelerometer->start())
                {
                    auto statusText = QString("Sensor %1 %2 started.").arg(type).arg(sensor.description());
                    emit sendInfo(statusText);
                }
            }
            else if(sensor_type.contains("Orientation"))
            {
                if (!sensorOrientation) {
                    sensorOrientation = new QOrientationSensor(this);
                    connect(sensorOrientation, &QOrientationSensor::readingChanged, this, &Sensors::orientation_changed);
                    sensorOrientation->setIdentifier(identifier);

                    if (!sensorOrientation->connectToBackend()) {
                        qDebug() << "Can't connect to Backend sensor: " + sensor_type;
                        statusText = QString("Can't connect to Backend sensor: %1").arg(sensor_type);
                        emit sendInfo(statusText);
                        delete sensorOrientation;
                        sensorOrientation = nullptr;
                        return;
                    }
                }

                if (sensorOrientation && sensorOrientation->start()) {
                    auto statusText = QString("Sensor %1 %2 started.").arg(type).arg(sensorOrientation->description());
                    emit sendInfo(statusText);
                }
            }
            else if(sensor_type.contains("AmbientLight"))
            {
                if (!sensorAmbientLight) {
                    sensorAmbientLight = new QAmbientLightSensor(this);
                    connect(sensorAmbientLight, &QAmbientLightSensor::readingChanged, this, &Sensors::ambientLight_changed);
                    sensorAmbientLight->setIdentifier(identifier);

                    if (!sensorAmbientLight->connectToBackend()) {
                        qDebug() << "Can't connect to Backend sensor: " + sensor_type;
                        statusText = QString("Can't connect to Backend sensor: %1").arg(sensor_type);
                        emit sendInfo(statusText);
                        delete sensorAmbientLight;
                        sensorAmbientLight = nullptr;
                        return;
                    }
                }

                if (sensorAmbientLight && sensorAmbientLight->start()) {
                    auto statusText = QString("Sensor %1 %2 started.").arg(type).arg(sensorAmbientLight->description());
                    emit sendInfo(statusText);
                }
            }
            else if(sensor_type.contains("Tilt"))
            {
                if (!sensorTilt) {
                    sensorTilt = new QTiltSensor(this);
                    connect(sensorTilt, &QAmbientLightSensor::readingChanged, this, &Sensors::tilt_changed);
                    sensorTilt->setIdentifier(identifier);

                    if (!sensorTilt->connectToBackend()) {
                        qDebug() << "Can't connect to Backend sensor: " + sensor_type;
                        statusText = QString("Can't connect to Backend sensor: %1").arg(sensor_type);
                        emit sendInfo(statusText);
                        delete sensorTilt;
                        sensorTilt = nullptr;
                        return;
                    }
                }

                if (sensorTilt && sensorTilt->start()) {
                    auto statusText = QString("Sensor %1 %2 started.").arg(type).arg(sensorTilt->description());
                    emit sendInfo(statusText);
                }
            }
        }
    }

#else
    qDebug() << "This code is meant for Android only.";
    emit sendInfo("This code is meant for Android only.");
#endif
}

void Sensors::stopSensors()
{
    if(sensorAccelerometer)
    {
        sensorAccelerometer->stop();
        delete sensorAccelerometer;
        sensorAccelerometer = nullptr;
    }
    if(sensorOrientation)
    {
        sensorOrientation->stop();
        delete sensorOrientation;
        sensorOrientation = nullptr;
    }

    if(sensorAmbientLight)
    {
        sensorAmbientLight->stop();
        delete sensorAmbientLight;
        sensorAmbientLight = nullptr;
    }

    if(sensorTilt)
    {
        sensorTilt->stop();
        delete sensorTilt;
        sensorTilt = nullptr;
    }
}

void Sensors::accelerometer_changed()
{
    static qint64 lastEmitTimestamp = 0; // Store the last emit timestamp
    qint64 currentTimestamp = QDateTime::currentMSecsSinceEpoch(); // Get the current timestamp in milliseconds

    // Check if at least 1 second has passed since the last emission
    if (currentTimestamp - lastEmitTimestamp >= 1000)
    {
        QAccelerometerReading * accelerometerReading = sensorAccelerometer->reading();
        if (accelerometerReading)
        {
            qreal x = accelerometerReading->property("x").value<qreal>();
            qreal y = accelerometerReading->property("y").value<qreal>();
            qreal z = accelerometerReading->property("z").value<qreal>();
            quint64 timestamp = sensorAccelerometer->reading()->timestamp();

            auto statusText = QString("time: %1 x: %2 y: %3 z: %4").arg(timestamp).arg(x).arg(y).arg(z);
            emit sendSensorValue(statusText);

            // Update the last emit timestamp
            lastEmitTimestamp = currentTimestamp;
        }
    }
}

void Sensors::orientation_changed()
{
        if (sensorOrientation && sensorOrientation->isActive()) {
            // Retrieve the orientation reading and process it
            QOrientationReading *reading = sensorOrientation->reading();
            if (reading) {
            }

        }
}

void Sensors::ambientLight_changed()
{
        if (sensorAmbientLight && sensorAmbientLight->isActive()) {
            // Retrieve the ambient light reading and process it
            QAmbientLightReading *reading = sensorAmbientLight->reading();
            if (reading) {
                qreal lux = reading->lightLevel();
//                emit sendInfo(QString::number(lux, 'f', 2));
            }
        }
}

void Sensors::tilt_changed()
{
        if (sensorTilt && sensorTilt->isActive()) {
            // Retrieve the ambient light reading and process it
            QTiltReading *reading = sensorTilt->reading();
            if (reading) {
            }
        }
}
