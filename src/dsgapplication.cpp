/*
 * Copyright (C) 2022 deepin Technology Co., Ltd.
 *
 * Author:     JiDeZhang <zhangjide@uniontech.com>
 *
 * Maintainer: JiDeZhang <zhangjide@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "dsgapplication.h"

#include <QByteArray>
#include <QCoreApplication>

DCORE_BEGIN_NAMESPACE

static inline QByteArray getSelfAppId() {
    // The env is set by the application starter(eg, org.desktopspec.ApplicationManager service)
    QByteArray selfId = qgetenv("DSG_APP_ID");
    if (!selfId.isEmpty())
        return selfId;
    selfId = DSGApplication::getId(QCoreApplication::applicationPid());
    if (!qEnvironmentVariableIsSet("DTK_DISABLED_FALLBACK_APPID")) {
        selfId = QCoreApplication::applicationName().toLocal8Bit();
    }
    Q_ASSERT(!selfId.isEmpty());
    if (selfId.isEmpty()) {
        qt_assert("The application ID is empty", __FILE__, __LINE__);
    }
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
