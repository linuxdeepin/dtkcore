// SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsgapplication.h"

#include <sys/syscall.h>
#include <unistd.h>

#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QLoggingCategory>

#include <dbus/dbus.h>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(dsgApp, "dtk.core.dsg")
#else
Q_LOGGING_CATEGORY(dsgApp, "dtk.core.dsg", QtInfoMsg)
#endif

DCORE_BEGIN_NAMESPACE

// D-Bus utility functions using libdbus-1
static bool checkDBusServiceActivatable(const QString &service)
{
    DBusError error;
    dbus_error_init(&error);

    DBusConnection *connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error)) {
        qCWarning(dsgApp) << "Failed to connect to session bus:" << error.message;
        dbus_error_free(&error);
        return false;
    }

    if (!connection) {
        qCWarning(dsgApp) << "Failed to get session bus connection";
        return false;
    }

    // Create method call to check if service is registered
    DBusMessage *msg = dbus_message_new_method_call(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "NameHasOwner"
    );

    if (!msg) {
        qCWarning(dsgApp) << "Failed to create D-Bus message";
        dbus_connection_unref(connection);
        return false;
    }

    const char *serviceName = service.toUtf8().constData();
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &serviceName, DBUS_TYPE_INVALID)) {
        qCWarning(dsgApp) << "Failed to append arguments to D-Bus message";
        dbus_message_unref(msg);
        dbus_connection_unref(connection);
        return false;
    }

    // Send message and get reply
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(connection, msg, 1000, &error);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&error)) {
        qCWarning(dsgApp) << "D-Bus call failed:" << error.message;
        dbus_error_free(&error);
        dbus_connection_unref(connection);
        return false;
    }

    if (!reply) {
        qCWarning(dsgApp) << "No reply received from D-Bus";
        dbus_connection_unref(connection);
        return false;
    }

    dbus_bool_t hasOwner = FALSE;
    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_BOOLEAN, &hasOwner, DBUS_TYPE_INVALID)) {
        qCWarning(dsgApp) << "Failed to parse D-Bus reply:" << error.message;
        dbus_error_free(&error);
        dbus_message_unref(reply);
        dbus_connection_unref(connection);
        return false;
    }

    dbus_message_unref(reply);

    if (!hasOwner) {
        dbus_connection_unref(connection);
        return false;
    }

    // Check if service is activatable
    msg = dbus_message_new_method_call(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "ListActivatableNames"
    );

    if (!msg) {
        qCWarning(dsgApp) << "Failed to create ListActivatableNames message";
        dbus_connection_unref(connection);
        return false;
    }

    reply = dbus_connection_send_with_reply_and_block(connection, msg, 1000, &error);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&error)) {
        qCWarning(dsgApp) << "ListActivatableNames call failed:" << error.message;
        dbus_error_free(&error);
        dbus_connection_unref(connection);
        return false;
    }

    if (!reply) {
        dbus_connection_unref(connection);
        return false;
    }

    DBusMessageIter iter, array_iter;
    dbus_message_iter_init(reply, &iter);

    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
        dbus_message_unref(reply);
        dbus_connection_unref(connection);
        return false;
    }

    dbus_message_iter_recurse(&iter, &array_iter);
    bool found = false;

    while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRING) {
        const char *name;
        dbus_message_iter_get_basic(&array_iter, &name);
        if (service == QString::fromUtf8(name)) {
            found = true;
            break;
        }
        dbus_message_iter_next(&array_iter);
    }

    dbus_message_unref(reply);
    dbus_connection_unref(connection);
    dbus_error_free(&error);

    return found;
}

static QByteArray callDBusIdentifyMethod(const QString &service, const QString &path, const QString &interface, int pidfd)
{
    DBusError error;
    dbus_error_init(&error);

    DBusConnection *connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error)) {
        qCWarning(dsgApp) << "Failed to connect to session bus:" << error.message;
        dbus_error_free(&error);
        return QByteArray();
    }

    if (!connection) {
        qCWarning(dsgApp) << "Failed to get session bus connection";
        return QByteArray();
    }

    // Create method call
    DBusMessage *msg = dbus_message_new_method_call(
        service.toUtf8().constData(),
        path.toUtf8().constData(),
        interface.toUtf8().constData(),
        "Identify"
    );

    if (!msg) {
        qCWarning(dsgApp) << "Failed to create D-Bus message";
        dbus_connection_unref(connection);
        return QByteArray();
    }

    // Append Unix file descriptor
    if (!dbus_message_append_args(msg, DBUS_TYPE_UNIX_FD, &pidfd, DBUS_TYPE_INVALID)) {
        qCWarning(dsgApp) << "Failed to append Unix FD to D-Bus message";
        dbus_message_unref(msg);
        dbus_connection_unref(connection);
        return QByteArray();
    }

    // Send message and get reply
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(connection, msg, 5000, &error);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&error)) {
        qCWarning(dsgApp) << "Identify from AM failed:" << error.message;
        dbus_error_free(&error);
        dbus_connection_unref(connection);
        return QByteArray();
    }

    if (!reply) {
        qCWarning(dsgApp) << "No reply received from Identify method";
        dbus_connection_unref(connection);
        return QByteArray();
    }

    const char *result;
    if (!dbus_message_get_args(reply, &error, DBUS_TYPE_STRING, &result, DBUS_TYPE_INVALID)) {
        qCWarning(dsgApp) << "Failed to parse Identify reply:" << error.message;
        dbus_error_free(&error);
        dbus_message_unref(reply);
        dbus_connection_unref(connection);
        return QByteArray();
    }

    QByteArray appId = QByteArray(result);
    qCInfo(dsgApp) << "AppId is fetched from AM, and value is " << appId;

    dbus_message_unref(reply);
    dbus_connection_unref(connection);
    dbus_error_free(&error);

    return appId;
}

static inline QByteArray getSelfAppId() {
    // The env is set by the application starter(eg, org.desktopspec.ApplicationManager service)
    QByteArray selfId = qgetenv("DSG_APP_ID");
    if (!selfId.isEmpty())
        return selfId;
    return DSGApplication::getId(QCoreApplication::applicationPid());
}

static bool isServiceActivatable(const QString &service)
{
    return checkDBusServiceActivatable(service);
}

// Format appId to valid.
static QByteArray formatAppId(const QByteArray &appId)
{
    static const QRegularExpression regex("[^\\w\\-\\.]");
    QString format(appId);
    format.replace(QDir::separator(), ".");
    format = format.replace(regex, QStringLiteral("-"));
    const QString InvalidPrefix{"."};
    if (format.startsWith(InvalidPrefix))
        format = format.mid(InvalidPrefix.size());
    return format.toLocal8Bit();
}

QByteArray DSGApplication::id()
{
    static QByteArray selfId = getSelfAppId();
    if (!selfId.isEmpty())
        return selfId;
    QByteArray result = selfId;
    if (!qEnvironmentVariableIsSet("DTK_DISABLED_FALLBACK_APPID")) {
        result = QCoreApplication::applicationName().toLocal8Bit();
        if (result.isEmpty()) {
            QFile file("/proc/self/cmdline");
            if (file.open(QIODevice::ReadOnly))
                result = file.readLine();
        }
        if (result.isEmpty()) {
            const QFileInfo file(QFile::symLinkTarget("/proc/self/exe"));
            if (file.exists())
                result = file.absoluteFilePath().toLocal8Bit();
        }
        if (!result.isEmpty()) {
            result = formatAppId(result);
            qCDebug(dsgApp) << "The applicatiion ID is fallback to " << result;
        }
    }
    if (result.isEmpty())
        qCWarning(dsgApp) << "The application ID is empty.";

    return result;
}

QByteArray DSGApplication::getId(qint64 pid)
{
    if (!isServiceActivatable("org.desktopspec.ApplicationManager1")) {
        qCInfo(dsgApp) << "Can't getId from AM for the " << pid << ", because AM is unavailable.";
        return QByteArray();
    }

    int pidfd = syscall(SYS_pidfd_open, pid, 0);
    if (pidfd < 0) {
        qCWarning(dsgApp) << "pidfd open failed:" << strerror(errno) << ", the pid:" << pid;
        return QByteArray();
    }

    QByteArray appId = callDBusIdentifyMethod("org.desktopspec.ApplicationManager1",
                                              "/org/desktopspec/ApplicationManager1",
                                              "org.desktopspec.ApplicationManager1",
                                              pidfd);
    // see QDBusUnixFileDescriptor: The original file descriptor is not touched and must be closed by the user.
    close(pidfd);

    return appId;
}

DCORE_END_NAMESPACE
