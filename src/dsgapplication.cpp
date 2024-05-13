// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsgapplication.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

DCORE_BEGIN_NAMESPACE

static inline QByteArray getSelfAppId() {
    // The env is set by the application starter(eg, org.desktopspec.ApplicationManager service)
    QByteArray selfId = qgetenv("DSG_APP_ID");
    if (!selfId.isEmpty())
        return selfId;
    selfId = DSGApplication::getId(QCoreApplication::applicationPid());
    if (selfId.isEmpty() && !qEnvironmentVariableIsSet("DTK_DISABLED_FALLBACK_APPID")) {
        auto appName = QCoreApplication::applicationName().toLocal8Bit();
        if (!appName.isEmpty()) {
            selfId = appName;
        } else {
#ifdef Q_OS_LINUX
            if (QFile::exists("/proc/self/exe")) {
                auto id = QFileInfo("/proc/self/exe").symLinkTarget().replace("/", ".");
                selfId = id.size() > 0 ? id.remove(0, 1).toLocal8Bit() : id.toLocal8Bit();
            }
#endif
        }
    }
    if (selfId.isEmpty()) {
        qWarning() << "The application ID is empty. " << __FILE__ << __LINE__;
    }

    Q_ASSERT(!selfId.isEmpty());
    return selfId;
}

QByteArray DSGApplication::id()
{
    static QByteArray selfId = getSelfAppId();
    return selfId;
}

QByteArray DSGApplication::getId(qint64)
{
    // TODO(zccrs): Call the org.desktopspec.ApplicationManager DBus service
    return nullptr;
}

DCORE_END_NAMESPACE
