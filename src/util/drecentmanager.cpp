/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * Maintainer: rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "drecentmanager.h"
#include <QMimeDatabase>
#include <QDomDocument>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QDebug>

DCORE_BEGIN_NAMESPACE

#define RECENT_PATH QDir::homePath() + "/.local/share/recently-used.xbel"

void DRecentManager::addItem(const QString &uri, DRecentData &data)
{
    QFile file(RECENT_PATH);

    if (!QFileInfo(uri).exists() || uri.isEmpty()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QString dateTime = QDateTime::currentDateTime().toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate);
    QDomDocument doc;

    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    // need to add file:// protocol.
    QUrl url(uri);
    url.setScheme("file");

    // get the MimeType name of the file.
    if (data.mimeType.isEmpty()) {
        data.mimeType = QMimeDatabase().mimeTypeForFile(uri).name();
    }

    QDomElement rootEle = doc.documentElement();
    QDomNodeList nodeList = rootEle.elementsByTagName("bookmark");
    QDomElement bookmarkEle;
    bool isFound = false;

    // find bookmark element exists.
    for (int i = 0; i < nodeList.size(); ++i) {
        const QString fileUrl = nodeList.at(i).toElement().attribute("href");

        if (fileUrl == url.toString()) {
            bookmarkEle = nodeList.at(i).toElement();
            isFound = true;
            break;
        }
    }

    // update element content.
    if (isFound) {
        QDomNodeList appList = bookmarkEle.elementsByTagName("bookmark:application");
        QDomElement appEle;
        bool appExists = false;

        for (int i = 0; i < appList.size(); ++i) {
            appEle = appList.at(i).toElement();

            if (appEle.attribute("name") == data.appName &&
                appEle.attribute("exec") == data.appExec) {
                appExists = true;
                break;
            }
        }

        if (appExists) {
            int count = appEle.attribute("count").toInt() + 1;
            bookmarkEle.setAttribute("modified", dateTime);
            bookmarkEle.setAttribute("visited", dateTime);
            appEle.setAttribute("modified", dateTime);
            appEle.setAttribute("count", QString::number(count));
        } else {
            QDomNode appsNode = bookmarkEle.elementsByTagName("bookmark:applications").at(0);

            appEle = doc.createElement("bookmark:application");
            appEle.setAttribute("name", data.appName);
            appEle.setAttribute("exec", data.appExec);
            appEle.setAttribute("modified", dateTime);
            appEle.setAttribute("count", "1");
            appsNode.toElement().appendChild(appEle);
        }
    }
    // add new elements if they don't exist.
    else {
        QDomElement bookmarkEle, infoEle, metadataEle, mimeEle, appsEle, appChildEle;

        bookmarkEle = doc.createElement("bookmark");
        bookmarkEle.setAttribute("href", url.toString());
        bookmarkEle.setAttribute("added", dateTime);
        bookmarkEle.setAttribute("modified", dateTime);
        bookmarkEle.setAttribute("visited", dateTime);

        infoEle = doc.createElement("info");
        bookmarkEle.appendChild(infoEle);

        metadataEle = doc.createElement("metadata");
        metadataEle.setAttribute("owner", "http://freedesktop.org");
        infoEle.appendChild(metadataEle);

        mimeEle = doc.createElement("mime:mime-type");
        mimeEle.setAttribute("type", data.mimeType);
        metadataEle.appendChild(mimeEle);

        appsEle = doc.createElement("bookmark:applications");
        appChildEle = doc.createElement("bookmark:application");
        appChildEle.setAttribute("name", data.appName);
        appChildEle.setAttribute("exec", data.appExec);
        appChildEle.setAttribute("modified", dateTime);
        appChildEle.setAttribute("count", "1");

        appsEle.appendChild(appChildEle);
        metadataEle.appendChild(appsEle);

        QDomNode result = rootEle.appendChild(bookmarkEle);
        if (result.isNull()) {
            return;
        }
    }

    // write to file.
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc.toString();
    out.flush();
    file.close();

    return;
}

void DRecentManager::removeItem(const QString &target)
{
    removeItems(QStringList() << target);
}

void DRecentManager::removeItems(const QStringList &list)
{
    QFile file(RECENT_PATH);

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomNodeList nodeList = rootEle.elementsByTagName("bookmark");

    for (int i = 0; i < nodeList.count(); ) {
        const QString fileUrl = nodeList.at(i).toElement().attribute("href");

        if (list.contains(fileUrl)) {
            rootEle.removeChild(nodeList.at(i));
        } else {
            ++i;
        }
    }

    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc.toString();
    out.flush();
    file.close();

    return;
}

DCORE_END_NAMESPACE
