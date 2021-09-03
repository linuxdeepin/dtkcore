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

#ifndef DRECENTMANAGER_H
#define DRECENTMANAGER_H

#include <dtkcore_global.h>
#include <QString>

DCORE_BEGIN_NAMESPACE

struct LIBDTKCORESHARED_EXPORT DRecentData
{
    QString appName;
    QString appExec;
    QString mimeType;
};

class LIBDTKCORESHARED_EXPORT DRecentManager
{
public:
    static bool addItem(const QString &uri, DRecentData &data);
    static void removeItem(const QString &target);
    static void removeItems(const QStringList &list);
};

DCORE_END_NAMESPACE

#endif // DRECENTMANAGER_H
