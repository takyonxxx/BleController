#include "gattserver.h"

#define SERVICEUUID   "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define RXUUID        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TXUUID        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

GattServer *GattServer::theInstance_= nullptr;
static QScopedPointer<QLowEnergyController> bleController;
static QHash<QBluetoothUuid, ServicePtr> services;
static QBluetoothAddress remoteDevice;

static bool m_ConnectionState = false;

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
    qRegisterMetaType<QLowEnergyController::ControllerState>();
    qRegisterMetaType<QLowEnergyController::Error>();
    qRegisterMetaType<QLowEnergyConnectionParameters>();
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
    remoteDevice = bleController.data()->remoteAddress();
    m_ConnectionState = true;
    emit connectionState(m_ConnectionState);
    auto statusText = QString("Connected to %1").arg(remoteDevice.toString());
    emit sendInfo(statusText);
}

void GattServer::handleDisconnected()
{
    m_ConnectionState = false;
    emit connectionState(m_ConnectionState);

    while (bleController->state() != QLowEnergyController::UnconnectedState) {
        bleController->disconnectFromDevice();
    }

    if(bleController->state() == QLowEnergyController::UnconnectedState)
    {
        auto statusText = QString("Disconnected from %1").arg(remoteDevice.toString());
        emit sendInfo(statusText);
    }

    reconnect();
}

void GattServer::addService(const QLowEnergyServiceData &serviceData)
{
    const ServicePtr service(bleController->addService(serviceData));
    Q_ASSERT(service);

    services.insert(service->serviceUuid(), service);
}
void GattServer::startBleService()
{
    bleController.reset(QLowEnergyController::createPeripheral());

    serviceData.setUuid(QBluetoothUuid(QUuid(SERVICEUUID)));
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);    

    QLowEnergyCharacteristicData charRxData;
    charRxData.setUuid(QBluetoothUuid(QUuid(RXUUID)));
    charRxData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify) ;
    charRxData.setValue(QByteArray(2, 0));
    const QLowEnergyDescriptorData rxClientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, QByteArray(2, 0));
    charRxData.addDescriptor(rxClientConfig);
    serviceData.addCharacteristic(charRxData);

    QLowEnergyCharacteristicData charTxData;
    charTxData.setUuid(QBluetoothUuid(QUuid(TXUUID)));
    charTxData.setValue(QByteArray(2, 0));
    charTxData.setProperties(QLowEnergyCharacteristic::Write);
    const QLowEnergyDescriptorData txClientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, QByteArray(2, 0));
    charTxData.addDescriptor(txClientConfig);
    serviceData.addCharacteristic(charTxData);

    addService(serviceData);

    const ServicePtr service = services.value(QBluetoothUuid(QUuid(SERVICEUUID)));
    Q_ASSERT(service);

    QObject::connect(bleController.data(), &QLowEnergyController::connected, this, &GattServer::handleConnected);
    QObject::connect(bleController.data(), &QLowEnergyController::disconnected, this, &GattServer::handleDisconnected);
    QObject::connect(bleController.data(), SIGNAL(error(QLowEnergyController::Error)), this, SLOT(controllerError(QLowEnergyController::Error)));


    QObject::connect(service.data(), &QLowEnergyService::characteristicChanged, this, &GattServer::onCharacteristicChanged);
    QObject::connect(service.data(), &QLowEnergyService::characteristicRead, this, &GattServer::onCharacteristicChanged);

    params.setMode(QLowEnergyAdvertisingParameters::AdvInd);
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);   
    advertisingData.setServices(services.keys());
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("Meta Quest2");
    bleController->startAdvertising(params, advertisingData);

    // We have to check if advertising succeeded ot not. If there was an advertising error we will
    // try to reinitialize our bluetooth service
    while (bleController->state()!= QLowEnergyController::AdvertisingState){
        stopBluetoothService();
        bleController->startAdvertising(params, advertisingData);
    }

    if(bleController->state()== QLowEnergyController::AdvertisingState)
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

void GattServer::reconnect()
{
    try{
        if(bleController->state()== QLowEnergyController::UnconnectedState){

            //bleController.reset(QLowEnergyController::createPeripheral());
            services.clear();
            bleController->services().clear();

            addService(serviceData);

            const ServicePtr service = services.value(QBluetoothUuid(QUuid(SERVICEUUID)));
            Q_ASSERT(service);

            QObject::connect(service.data(), &QLowEnergyService::characteristicChanged, this, &GattServer::onCharacteristicChanged);
            QObject::connect(service.data(), &QLowEnergyService::characteristicRead, this, &GattServer::onCharacteristicChanged);

            bleController->startAdvertising(params, advertisingData);

            // We have to check if advertising succeeded ot not. If there was an advertising error we will
            // try to reinitialize our bluetooth service
            while (bleController->state()!= QLowEnergyController::AdvertisingState){
                stopBluetoothService();
                bleController->startAdvertising(params, advertisingData);
            }

            if(bleController->state()== QLowEnergyController::AdvertisingState)
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

        if(bleController->state()==QLowEnergyController::AdvertisingState)
            bleController->stopAdvertising();

        if(bleController->state()==QLowEnergyController::UnconnectedState)
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
