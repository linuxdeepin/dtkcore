/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <zhangdingyuan@deepin.com>
 *
 * Maintainer: justforlxz <zhangdingyuan@deepin.com>
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

#include "dnotifysender.h"
#include "ddbussender.h"

DCORE_BEGIN_NAMESPACE

namespace DUtil {

struct DNotifyData {
    uint        m_replaceId;
    int         m_timeOut;
    QString     m_body;
    QString     m_summary;
    QString     m_appIcon;
    QString     m_appName;
    QStringList m_actions;
    QVariantMap m_hints;
};

DNotifySender::DNotifySender(const QString &summary) : m_dbusData(std::make_shared<DNotifyData>())
{
    m_dbusData->m_summary = summary;
}

DNotifySender DNotifySender::appName(const QString &appName)
{
    m_dbusData->m_appName = appName;

    return *this;
}

DNotifySender DNotifySender::appIcon(const QString &appIcon)
{
    m_dbusData->m_appIcon = appIcon;

    return *this;
}

DNotifySender DNotifySender::appBody(const QString &appBody)
{
    m_dbusData->m_body = appBody;

    return *this;
}

DNotifySender DNotifySender::replaceId(const uint replaceId)
{
    m_dbusData->m_replaceId = replaceId;

    return *this;
}

DNotifySender DNotifySender::timeOut(const int timeOut)
{
    m_dbusData->m_timeOut = timeOut;

    return *this;
}

DNotifySender DNotifySender::actions(const QStringList &actions)
{
    m_dbusData->m_actions = actions;

    return *this;
}

DNotifySender DNotifySender::hints(const QVariantMap &hints)
{
    m_dbusData->m_hints = hints;

    return *this;
}

QDBusPendingCall DNotifySender::call()
{
    return DDBusSender()
        .service("org.freedesktop.Notifications")
        .path("/org/freedesktop/Notifications")
        .interface("org.freedesktop.Notifications")
        .method(QString("Notify"))
        .arg(m_dbusData->m_appName)
        .arg(m_dbusData->m_replaceId)
        .arg(m_dbusData->m_appIcon)
        .arg(m_dbusData->m_summary)
        .arg(m_dbusData->m_body)
        .arg(m_dbusData->m_actions)
        .arg(m_dbusData->m_hints)
        .arg(m_dbusData->m_timeOut)
        .call();
}

}  // namespace DUtil

DCORE_END_NAMESPACE
