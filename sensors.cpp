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
            auto statusText = QString("Sensor found %1 %2").arg(type).arg(sensor.description());
            emit sendInfo(statusText);

            QString sensor_type = type;
            if(sensor_type.contains("Accelerometer"))
            {
                if(!sensorAcc)
                {
                    sensorAcc = new QAccelerometer(this);
                    connect(sensorAcc, SIGNAL(readingChanged()), this, SLOT(accelerometer_changed()));
                    sensorAcc->setIdentifier(identifier);

                    if (!sensorAcc->connectToBackend()) {
                        qDebug() << "Can't connect to Backend sensor: " + sensor_type;
                        delete sensorAcc;
                        sensorAcc = 0;
                        return;
                    }
                }

                if(sensorAcc && sensorAcc->start())
                {
                    auto statusText = QString("Sensor %1 %2 started.").arg(type).arg(sensor.description());
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
    if(sensorAcc)
        sensorAcc->stop();
}

void Sensors::accelerometer_changed()
{
    static qint64 lastEmitTimestamp = 0; // Store the last emit timestamp
    qint64 currentTimestamp = QDateTime::currentMSecsSinceEpoch(); // Get the current timestamp in milliseconds

    // Check if at least 1 second has passed since the last emission
    if (currentTimestamp - lastEmitTimestamp >= 1000)
    {
        accelerometerReading = sensorAcc->reading();
        if (accelerometerReading)
        {
            qreal x = accelerometerReading->property("x").value<qreal>();
            qreal y = accelerometerReading->property("y").value<qreal>();
            qreal z = accelerometerReading->property("z").value<qreal>();
            quint64 timestamp = sensorAcc->reading()->timestamp();

            auto statusText = QString("time: %1 x: %2 y: %3 z: %4").arg(timestamp).arg(x).arg(y).arg(z);
            emit sendSensorValue(statusText);

            // Update the last emit timestamp
            lastEmitTimestamp = currentTimestamp;
        }
    }
}
