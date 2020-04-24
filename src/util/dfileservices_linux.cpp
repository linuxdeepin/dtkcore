/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>
#include <QFile>

#include "dfileservices.h"

DCORE_BEGIN_NAMESPACE

#define EASY_CALL_DBUS(name)\
    QDBusInterface *interface = fileManager1DBusInterface();\
    return interface && interface->call(#name, urls2uris(urls), startupId).type() != QDBusMessage::ErrorMessage;

static QDBusInterface *fileManager1DBusInterface()
{
    static QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                                    QStringLiteral("/org/freedesktop/FileManager1"),
                                    QStringLiteral("org.freedesktop.FileManager1"));
    return &interface;
}

static QStringList urls2uris(const QList<QUrl> &urls)
{
    QStringList list;

    list.reserve(urls.size());

    for (const QUrl &url : urls) {
        list << url.toString();
    }

    return list;
}

static QList<QUrl> path2urls(const QList<QString> &paths)
{
    QList<QUrl> list;

    list.reserve(paths.size());

    for (const QString &path : paths) {
        list << QUrl::fromLocalFile(path);
    }

    return list;
}

bool DFileServices::showFolder(QString localFilePath, const QString &startupId)
{
    return showFolder(QUrl::fromLocalFile(localFilePath), startupId);
}

bool DFileServices::showFolders(const QList<QString> localFilePaths, const QString &startupId)
{
    return showFolders(path2urls(localFilePaths), startupId);
}

bool DFileServices::showFolder(QUrl url, const QString &startupId)
{
    return showFolders(QList<QUrl>() << url, startupId);
}

bool DFileServices::showFolders(const QList<QUrl> urls, const QString &startupId)
{
    EASY_CALL_DBUS(ShowFolders)
}

bool DFileServices::showFileItemPropertie(QString localFilePath, const QString &startupId)
{
    return showFileItemPropertie(QUrl::fromLocalFile(localFilePath), startupId);
}

bool DFileServices::showFileItemProperties(const QList<QString> localFilePaths, const QString &startupId)
{
    return showFileItemProperties(path2urls(localFilePaths), startupId);
}

bool DFileServices::showFileItemPropertie(QUrl url, const QString &startupId)
{
    return showFileItemProperties(QList<QUrl>() << url, startupId);
}

bool DFileServices::showFileItemProperties(const QList<QUrl> urls, const QString &startupId)
{
    EASY_CALL_DBUS(ShowItemProperties)
}

bool DFileServices::showFileItem(QString localFilePath, const QString &startupId)
{
    return showFileItem(QUrl::fromLocalFile(localFilePath), startupId);
}

bool DFileServices::showFileItems(const QList<QString> localFilePaths, const QString &startupId)
{
    return showFileItems(path2urls(localFilePaths), startupId);
}

bool DFileServices::showFileItem(QUrl url, const QString &startupId)
{
    return showFileItems(QList<QUrl>() << url, startupId);
}

bool DFileServices::showFileItems(const QList<QUrl> urls, const QString &startupId)
{
    EASY_CALL_DBUS(ShowItems)
}

bool DFileServices::trash(QString localFilePath)
{
    return trash(QUrl::fromLocalFile(localFilePath));
}

bool DFileServices::trash(const QList<QString> localFilePaths)
{
    return trash(path2urls(localFilePaths));
}

bool DFileServices::trash(QUrl url)
{
    return trash(QList<QUrl>() << url);
}

bool DFileServices::trash(const QList<QUrl> urls)
{
    QDBusInterface *interface = fileManager1DBusInterface();
    return interface && interface->call("Trash", urls2uris(urls)).type() != QDBusMessage::ErrorMessage;
}

QString DFileServices::errorMessage()
{
    return fileManager1DBusInterface()->lastError().message();
}

DCORE_END_NAMESPACE
