/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * Maintainer: rekols <rekols@foxmail.com>
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

#include "drecentmanager.h"
#include <QMimeDatabase>
#include <QDomDocument>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>

DCORE_BEGIN_NAMESPACE

#define RECENT_PATH QDir::homePath() + "/.local/share/recently-used.xbel"

/*!
  \class Dtk::Core::DRecentManager
  \inmodule dtkcore
  
  \brief DRecentManager 是用来管理最近文件列表的类，提供了添加与删除文件项.
  
  遵循 freedesktop 标准，在本地 share 目录存放，文件名为: recently-used.xbel，所以每个用户都有不同的列表。
  该类的存在就是为 deepin 应用提供一个工具类，方便让打开的文件添加到最近文件列表中。

  \sa Dtk::Core::DRecentData
 */

/*!
  \class Dtk::Core::DRecentData
  \inmodule dtkcore

  \brief 文件信息结构体.
  
  \table
  \row
    \li appName
    \li 应用名称
  \row
    \li appExec
    \li 应用命令行名称
  \row
    \li mimeType
    \li 文件 mimetype 名称，一般不需要填写，DRecentManager 内部自动获取
  \endtable
  \sa Dtk::Core::DRecentManager
 */

/*!
  \brief DRecentManager::addItem 在最近列表中添加一个项.
  \a uri 文件路径
  \a data 数据信息
  \return 如果返回 true 则成功添加，false 为添加失败
 */

bool DRecentManager::addItem(const QString &uri, DRecentData &data)
{
    if (!QFileInfo(uri).exists() || uri.isEmpty()) {
        return false;
    }

    QFile file(RECENT_PATH);
    file.open(QIODevice::ReadWrite | QIODevice::Text);

    QString dateTime = QDateTime::currentDateTime().toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate);
    QDomDocument doc;

    if (!doc.setContent(&file)) {
        doc.clear();
        doc.appendChild(doc.createProcessingInstruction("xml","version=\'1.0\' encoding=\'utf-8\'"));
        QDomElement xbelEle = doc.createElement("xbel");
        xbelEle.setAttribute("xmlns:mime", "http://www.freedesktop.org/standards/shared-mime-info");
        xbelEle.setAttribute("version", "1.0");
        xbelEle.setAttribute("xmlns:bookmark", "http://www.freedesktop.org/standards/desktop-bookmarks");
        doc.appendChild(xbelEle);
    }
    file.close();

    // need to add file:// protocol.
    QUrl url = QUrl::fromLocalFile(uri);

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

        if (fileUrl == url.toEncoded(QUrl::FullyDecoded)) {
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
        QString hrefStr = url.toEncoded(QUrl::FullyEncoded);

        bookmarkEle = doc.createElement("bookmark");
        bookmarkEle.setAttribute("href", hrefStr);
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
            return false;
        }
    }

    // write to file.
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc.toString();
    out.flush();
    file.close();

    return true;
}

/*!
  \brief DRecentManager::removeItem 在最近列表中移除单个文件路径
  \a target 需要移除的文件路径
 */

void DRecentManager::removeItem(const QString &target)
{
    removeItems(QStringList() << target);
}

/*!
  \brief DRecentManager::removeItem 在最近列表中移除多个文件路径
  \a list 需要移除的文件路径列表
 */

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

        if (list.contains(QUrl::fromPercentEncoding(fileUrl.toLatin1())) ||
            list.contains(QUrl(fileUrl).toEncoded(QUrl::FullyDecoded))) {
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
