// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "fakedbusservice.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
FakeDBusService::FakeDBusService(QObject *parent)
    : QObject(parent)
    , m_strproperty("testDBusService")
    , m_objectpaths({QDBusObjectPath("/ss"), QDBusObjectPath("/bb")})
{
    registerService();
}

FakeDBusService::~FakeDBusService()
{
    unregisterService();
}

void FakeDBusService::registerService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bool serviceOk = bus.registerService(m_service);
    if (!serviceOk) {
        QString errorMsg = bus.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";
        qWarning() << QString("Can't register the %1 service, %2.").arg(m_service).arg(errorMsg);
    }
    bool objectOk = bus.registerObject(m_path, this, QDBusConnection::ExportScriptableContents);
    if (!objectOk) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(m_path);
    }
    m_registered = serviceOk && objectOk;
}

void FakeDBusService::unregisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(m_path);
    bus.unregisterService(m_service);
}

FakeDBusServiceParent::FakeDBusServiceParent(QObject *parent)
    : QObject(parent)
{
    m_dDbusFakeDBusServiceInter = new DDBusInterface(FakeDBusService::get_service(),
                                                     FakeDBusService::get_path(),
                                                     FakeDBusService::get_interface(),
                                                     QDBusConnection::sessionBus(),
                                                     this);
}

QList<QDBusObjectPath> FakeDBusServiceParent::objectpaths()
{
    return qvariant_cast<QList<QDBusObjectPath>>(m_dDbusFakeDBusServiceInter->property("objectPaths"));
}
