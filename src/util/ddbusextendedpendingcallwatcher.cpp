// SPDX-FileCopyrightText: 2015 Jolla Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "ddbusextendedpendingcallwatcher_p.h"


DDBusExtendedPendingCallWatcher::DDBusExtendedPendingCallWatcher(const QDBusPendingCall &call, const QString &asyncProperty, const QVariant &previousValue, QObject *parent)
    : QDBusPendingCallWatcher(call, parent)
    , m_asyncProperty(asyncProperty)
    , m_previousValue(previousValue)
{
}

DDBusExtendedPendingCallWatcher::~DDBusExtendedPendingCallWatcher()
{
}
