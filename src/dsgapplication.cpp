// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsgapplication.h"

#include <sys/syscall.h>
#include <unistd.h>

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QDBusUnixFileDescriptor>
#include <QDBusReply>
#include <QRegularExpression>
#include <QLoggingCategory>

#include <DDBusInterface>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(dsgApp, "dtk.core.dsg")
#else
Q_LOGGING_CATEGORY(dsgApp, "dtk.core.dsg", QtInfoMsg)
#endif

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

QByteArray DSGApplication::getId(qint64 pid)
{
    int pidfd = syscall(SYS_pidfd_open, pid, 0);
    if (pidfd < 0) {
        qCWarning(dsgApp) << "pidfd open failed:" << strerror(errno);
        return QByteArray();
    }

    DDBusInterface infc("org.desktopspec.ApplicationManager1",
                        "/org/desktopspec/ApplicationManager1",
                        "org.desktopspec.ApplicationManager1");

    QDBusReply<QString> reply = infc.call("Identify", QVariant::fromValue(QDBusUnixFileDescriptor(pidfd)));

    if (!reply.isValid()) {
        qCWarning(dsgApp) << "Identify from AM failed." << reply.error().message();
        return QByteArray();
    }

    return reply.value().toLatin1();
}

DCORE_END_NAMESPACE
