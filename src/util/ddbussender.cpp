#include "ddbussender.h"

#include <QDBusInterface>
#include <QDebug>

DDBusSender::DDBusSender()
    : m_dbusData(std::make_shared<DDBusData>())
{
}

DDBusSender DDBusSender::service(const QString &service)
{
    m_dbusData->service = service;

    return *this;
}

DDBusSender DDBusSender::interface(const QString &interface)
{
    m_dbusData->interface = interface;

    return *this;
}

DDBusCaller DDBusSender::method(const QString &method)
{
    return DDBusCaller(method, m_dbusData);
}

DDBusProperty DDBusSender::property(const QString &property)
{
    return DDBusProperty(property, m_dbusData);
}

DDBusSender DDBusSender::path(const QString &path)
{
    m_dbusData->path = path;

    return *this;
}

DDBusSender DDBusSender::type(const QDBusConnection::BusType busType)
{
    switch (busType)
    {
    case QDBusConnection::SessionBus:
        m_dbusData->connection = QDBusConnection::sessionBus();
        break;

    case QDBusConnection::SystemBus:
        m_dbusData->connection = QDBusConnection::systemBus();
        break;

    default:
        Q_UNREACHABLE_IMPL();
    }

    return *this;
}

DDBusData::DDBusData()
    : connection(QDBusConnection::sessionBus())
{

}

QDBusPendingCall DDBusCaller::call()
{
    QDBusInterface iface(m_dbusData->service, m_dbusData->path, m_dbusData->interface, m_dbusData->connection);

    return iface.asyncCallWithArgumentList(m_methodName, m_arguments);
}

DDBusCaller::DDBusCaller(const QString &method, std::shared_ptr<DDBusData> data)
    : m_dbusData(data)
    , m_methodName(method)
{
}

QDBusPendingCall DDBusProperty::get()
{
    QDBusInterface iface(m_dbusData->service, m_dbusData->path, QStringLiteral("org.freedesktop.DBus.Properties"), m_dbusData->connection);

    return iface.asyncCallWithArgumentList(QStringLiteral("Get"), { QVariant::fromValue(m_dbusData->interface), QVariant::fromValue(m_propertyName) });
}

DDBusProperty::DDBusProperty(const QString &property, std::shared_ptr<DDBusData> data)
    : m_dbusData(data)
    , m_propertyName(property)
{
}
