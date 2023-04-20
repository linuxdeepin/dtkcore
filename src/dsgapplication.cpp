// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsgapplication.h"

#include <QByteArray>
#include <QCoreApplication>

DCORE_BEGIN_NAMESPACE

static inline QByteArray getSelfAppId() {
    // The env is set by the application starter(eg, org.desktopspec.ApplicationManager service)
    QByteArray selfId = qgetenv("DSG_APP_ID");
    if (!selfId.isEmpty())
        return selfId;
    return DSGApplication::getId(QCoreApplication::applicationPid());
}

QByteArray DSGApplication::id()
{
    static QByteArray selfId = getSelfAppId();
    if (!selfId.isEmpty())
        return selfId;
    QByteArray result = selfId;
    if (!qEnvironmentVariableIsSet("DTK_DISABLED_FALLBACK_APPID")) {
        result = QCoreApplication::applicationName().toLocal8Bit();
    }
    Q_ASSERT(!result.isEmpty());
    if (result.isEmpty()) {
        qt_assert("The application ID is empty", __FILE__, __LINE__);
    }

    return result;
}

QByteArray DSGApplication::getId(qint64)
{
    // TODO(zccrs): Call the org.desktopspec.ApplicationManager DBus service
    return nullptr;
}

DCORE_END_NAMESPACE
