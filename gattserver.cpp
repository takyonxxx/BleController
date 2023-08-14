#include "gattserver.h"

#define SERVICEUUID   "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define RXUUID        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TXUUID        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

GattServer *GattServer::theInstance_= nullptr;

GattServer* GattServer::getInstance()
{
    if (theInstance_ == nullptr)
    {
        theInstance_ = new GattServer();
    }
    return theInstance_;
}

GattServer::GattServer(QObject *parent) : QObject(parent)
{
//    qRegisterMetaType<QLowEnergyController::ControllerState>();
//    qRegisterMetaType<QLowEnergyController::Error>();
//    qRegisterMetaType<QLowEnergyConnectionParameters>();

    //! [Advertising Data]
    QLowEnergyAdvertisingData advertisingData;
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("HeartRateServer");
    advertisingData.setServices(QList<QBluetoothUuid>() << QBluetoothUuid::ServiceClassUuid::HeartRate);
    //! [Advertising Data]

    //! [Service Data]
    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
    charData.setValue(QByteArray(2, 0));
    charData.setProperties(QLowEnergyCharacteristic::Notify);
    const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                QByteArray(2, 0));
    charData.addDescriptor(clientConfig);

    QLowEnergyServiceData serviceData;
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::HeartRate);
    serviceData.addCharacteristic(charData);
    //! [Service Data]

    //! [Start Advertising]
    bool errorOccurred = false;
    const std::unique_ptr<QLowEnergyController> leController(QLowEnergyController::createPeripheral());
    auto errorHandler = [&leController, &errorOccurred](QLowEnergyController::Error errorCode) {
        qWarning().noquote().nospace() << errorCode << " occurred: "
                                       << leController->errorString();
        if (errorCode != QLowEnergyController::RemoteHostClosedError) {
            qWarning("Heartrate-server quitting due to the error.");
            errorOccurred = true;
            QCoreApplication::quit();
        }
    };
    QObject::connect(leController.get(), &QLowEnergyController::errorOccurred, errorHandler);

    std::unique_ptr<QLowEnergyService> service(leController->addService(serviceData));
    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                   advertisingData);
    if (errorOccurred)
        return;
    //! [Start Advertising]

    //! [Provide Heartbeat]
    QTimer heartbeatTimer;
    quint8 currentHeartRate = 60;
    enum ValueChange { ValueUp, ValueDown } valueChange = ValueUp;
    const auto heartbeatProvider = [&service, &currentHeartRate, &valueChange]() {
        QByteArray value;
        value.append(char(0)); // Flags that specify the format of the value.
        value.append(char(currentHeartRate)); // Actual value.
        QLowEnergyCharacteristic characteristic
            = service->characteristic(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
        Q_ASSERT(characteristic.isValid());
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
        if (currentHeartRate == 60)
            valueChange = ValueUp;
        else if (currentHeartRate == 100)
            valueChange = ValueDown;
        if (valueChange == ValueUp)
            ++currentHeartRate;
        else
            --currentHeartRate;
    };
    QObject::connect(&heartbeatTimer, &QTimer::timeout, heartbeatProvider);
    heartbeatTimer.start(1000);
    //! [Provide Heartbeat]

    auto reconnect = [&leController, advertisingData, &service, serviceData]() {
        service.reset(leController->addService(serviceData));
        if (service) {
            leController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                           advertisingData, advertisingData);
        }
    };
    QObject::connect(leController.get(), &QLowEnergyController::disconnected, reconnect);

}

GattServer::~GattServer()
{
}

void GattServer::controllerError(QLowEnergyController::Error error)
{
    auto statusText = QString("Controller Error: %1").arg(error);
    emit sendInfo(statusText);
}

void GattServer::handleConnected()
{
    remoteDevice = leController.data()->remoteAddress();
    m_ConnectionState = true;
    emit connectionState(m_ConnectionState);
    auto statusText = QString("Connected to %1").arg(remoteDevice.toString());
    emit sendInfo(statusText);
}

void GattServer::handleDisconnected()
{
    m_ConnectionState = false;
    emit connectionState(m_ConnectionState);

    while (leController->state() != QLowEnergyController::UnconnectedState) {
        leController->disconnectFromDevice();
    }

    if(leController->state() == QLowEnergyController::UnconnectedState)
    {
        auto statusText = QString("Disconnected from %1").arg(remoteDevice.toString());
        emit sendInfo(statusText);
    }

    reconnect();
}

void GattServer::addService(const QLowEnergyServiceData &serviceData)
{
    const ServicePtr service(leController->addService(serviceData));
    Q_ASSERT(service);

    services.insert(service->serviceUuid(), service);
}

void GattServer::startBleService()
{
//        leController.reset(QLowEnergyController::createPeripheral());

//        //! [Service Data Rx]
//        QLowEnergyCharacteristicData charRxData;
//        charRxData.setUuid(QBluetoothUuid(QUuid(RXUUID)));
//        charRxData.setValue(QByteArray(2, 0));
//        charRxData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify) ;
//        const QLowEnergyDescriptorData clientConfigRx(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
//                                                    QByteArray(2, 0));
//        charRxData.addDescriptor(clientConfigRx);

//        //! [Service Data Tx]
//        QLowEnergyCharacteristicData charTxData;
//        charTxData.setUuid(QBluetoothUuid(QUuid(TXUUID)));
//        charTxData.setValue(QByteArray(2, 0));
//        charTxData.setProperties(QLowEnergyCharacteristic::Write);
//        const QLowEnergyDescriptorData clientConfigTx(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
//                                                    QByteArray(2, 0));
//        charTxData.addDescriptor(clientConfigTx);


//        QLowEnergyServiceData serviceData;
//        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
//        serviceData.setUuid(QBluetoothUuid(QUuid(SERVICEUUID)));
//        serviceData.addCharacteristic(charTxData);
//        serviceData.addCharacteristic(charRxData);


//        QLowEnergyAdvertisingData advertisingData;
//        advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
//        advertisingData.setIncludePowerLevel(true);
//        advertisingData.setLocalName("Quest 2");
//        QList<QBluetoothUuid> serviceUuids;
//        serviceUuids << QBluetoothUuid(QUuid(TXUUID)); // Add your TXUUID here
//        serviceUuids << QBluetoothUuid(QUuid(RXUUID)); // Add your RXUUID here

//        advertisingData.setServices(serviceUuids);

//        QObject::connect(leController.data(), SIGNAL(error(QLowEnergyController::Error)), this, SLOT(controllerError(QLowEnergyController::Error)));
//        QObject::connect(leController.data(), &QLowEnergyController::connected, this, &GattServer::handleConnected);
//        QObject::connect(leController.data(), &QLowEnergyController::disconnected, this, &GattServer::handleDisconnected);


//        QScopedPointer<QLowEnergyService> service(leController->addService(serviceData));
//        QObject::connect(service.data(), &QLowEnergyService::characteristicChanged, this, &GattServer::onCharacteristicChanged);
//        QObject::connect(service.data(), &QLowEnergyService::characteristicRead, this, &GattServer::onCharacteristicChanged);
//        QObject::connect(service.data(), &QLowEnergyService::characteristicWritten, this, &GattServer::onCharacteristicChanged);


//        leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData, advertisingData);

//        if(leController->state()== QLowEnergyController::AdvertisingState)
//        {
//            auto statusText = QString("Listening for Ble connection %1").arg(advertisingData.localName());
//            emit sendInfo(statusText);
//        }
//        else
//        {
//            auto statusText = QString("Ble connection can not start for %1").arg(advertisingData.localName());
//            emit sendInfo(statusText);
//        }

}


//void GattServer::startBleService()
//{
//    leController.reset(QLowEnergyController::createPeripheral());

//    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
//    serviceData.setUuid(QBluetoothUuid(QUuid(SERVICEUUID)));

//    QLowEnergyCharacteristicData charRxData;
//    charRxData.setUuid(QBluetoothUuid(QUuid(RXUUID)));
//    charRxData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify) ;
//    charRxData.setValue(QByteArray(2, 0));
//    const QLowEnergyDescriptorData rxClientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, QByteArray(2, 0));
//    charRxData.addDescriptor(rxClientConfig);
//    serviceData.addCharacteristic(charRxData);

//    QLowEnergyCharacteristicData charTxData;
//    charTxData.setUuid(QBluetoothUuid(QUuid(TXUUID)));
//    charTxData.setValue(QByteArray(2, 0));
//    charTxData.setProperties(QLowEnergyCharacteristic::Write);
//    const QLowEnergyDescriptorData txClientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, QByteArray(2, 0));
//    charTxData.addDescriptor(txClientConfig);
//    serviceData.addCharacteristic(charTxData);

//    addService(serviceData);

//    const ServicePtr service = services.value(QBluetoothUuid(QUuid(SERVICEUUID)));
//    Q_ASSERT(service);

//    QObject::connect(leController.data(), &QLowEnergyController::connected, this, &GattServer::handleConnected);
//    QObject::connect(leController.data(), &QLowEnergyController::disconnected, this, &GattServer::handleDisconnected);
//    QObject::connect(leController.data(), SIGNAL(error(QLowEnergyController::Error)), this, SLOT(controllerError(QLowEnergyController::Error)));

//    QObject::connect(service.data(), &QLowEnergyService::characteristicChanged, this, &GattServer::onCharacteristicChanged);
//    QObject::connect(service.data(), &QLowEnergyService::characteristicRead, this, &GattServer::onCharacteristicChanged);

//    params.setMode(QLowEnergyAdvertisingParameters::AdvInd);
//    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
//    advertisingData.setServices(services.keys());
//    advertisingData.setIncludePowerLevel(true);
//    advertisingData.setLocalName("Quest2");

//    // We have to check if advertising succeeded ot not. If there was an advertising error we will
//    // try to reinitialize our bluetooth service
//    while (leController->state()!= QLowEnergyController::AdvertisingState){
//        stopBluetoothService();
//        leController->startAdvertising(params, advertisingData, advertisingData);
//    }

//    if(leController->state()== QLowEnergyController::AdvertisingState)
//    {
//        auto statusText = QString("Listening for Ble connection %1").arg(advertisingData.localName());
//        emit sendInfo(statusText);
//    }
//    else
//    {
//        auto statusText = QString("Ble connection can not start for %1").arg(advertisingData.localName());
//        emit sendInfo(statusText);
//    }
//}

void GattServer::reconnect()
{
    try{
        if(leController->state()== QLowEnergyController::UnconnectedState){

            //leController.reset(QLowEnergyController::createPeripheral());
            services.clear();
            leController->services().clear();

            addService(serviceData);

            const ServicePtr service = services.value(QBluetoothUuid(QUuid(SERVICEUUID)));
            Q_ASSERT(service);

            QObject::connect(service.data(), &QLowEnergyService::characteristicChanged, this, &GattServer::onCharacteristicChanged);
            QObject::connect(service.data(), &QLowEnergyService::characteristicRead, this, &GattServer::onCharacteristicChanged);

            leController->startAdvertising(params, advertisingData);

            // We have to check if advertising succeeded ot not. If there was an advertising error we will
            // try to reinitialize our bluetooth service
            while (leController->state()!= QLowEnergyController::AdvertisingState){
                stopBluetoothService();
                leController->startAdvertising(params, advertisingData);
            }

            if(leController->state()== QLowEnergyController::AdvertisingState)
            {
                auto statusText = QString("Listening for Ble connection %1").arg(advertisingData.localName());
                emit sendInfo(statusText);
            }
            else
            {
                auto statusText = QString("Ble connection can not start for %1").arg(advertisingData.localName());
                emit sendInfo(statusText);
            }
        }
    }
    catch(std::exception e){
        qDebug() << "Error reconnect: " << e.what();
    }
}


void GattServer::stopBluetoothService()
{
    try{

        if(leController->state()==QLowEnergyController::AdvertisingState)
            leController->stopAdvertising();

        if(leController->state()==QLowEnergyController::UnconnectedState)
        {
            auto statusText = QString("Ble connection stopped for %1").arg(advertisingData.localName());
            emit sendInfo(statusText);
        }
        else
        {
            auto statusText = QString("Ble connection can not stopped for %1").arg(advertisingData.localName());
            emit sendInfo(statusText);
        }

    }
    catch(std::exception e){
    }
}

void GattServer::readValue()
{
    const ServicePtr service = services.value(QBluetoothUuid(QUuid(SERVICEUUID)));
    Q_ASSERT(service);

    QLowEnergyCharacteristic cCharacteristic = service->characteristic(QBluetoothUuid(QUuid(TXUUID)));
    Q_ASSERT(cCharacteristic.isValid());
    service->readCharacteristic(cCharacteristic);
}

void GattServer::writeValue(const QByteArray &value)
{
    const ServicePtr service = services.value(QBluetoothUuid(QUuid(SERVICEUUID)));
    Q_ASSERT(service);

    QLowEnergyCharacteristic cCharacteristic = service->characteristic(QBluetoothUuid(QUuid(RXUUID)));
    Q_ASSERT(cCharacteristic.isValid());
    service->writeCharacteristic(cCharacteristic, value);
}

void GattServer::onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    Q_UNUSED(c)
    emit dataReceived(value);
}
