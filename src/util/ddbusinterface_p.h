// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "ddbusinterface.h"

class QDBusPendingCallWatcher;

class DDBusInterfacePrivate : public QObject
{
    Q_OBJECT

public:
    explicit DDBusInterfacePrivate(DDBusInterface *interface, QObject *parent);
    void updateProp(const char *propname, const QVariant &value);
    void initDBusConnection();
    void setServiceValid(bool valid);

private Q_SLOTS:
    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void onAsyncPropertyFinished(QDBusPendingCallWatcher *w);
    void onDBusNameHasOwner(bool valid);
    void onDBusNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOWner);

public:
    QObject *m_parent;
    QString m_suffix;
    QVariantMap m_propertyMap;
    bool m_serviceValid;

    DDBusInterface *q_ptr;
    Q_DECLARE_PUBLIC(DDBusInterface)
};

