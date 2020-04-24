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

#ifndef DBASEFILEWATCHER_P_H
#define DBASEFILEWATCHER_P_H

#include "base/private/dobject_p.h"

#include <QUrl>

DCORE_BEGIN_NAMESPACE

class DBaseFileWatcher;
class DBaseFileWatcherPrivate : public DObjectPrivate
{
public:
    DBaseFileWatcherPrivate(DBaseFileWatcher *qq);

    virtual bool start() = 0;
    virtual bool stop() = 0;

    QUrl url;
    bool started = false;
    static QList<DBaseFileWatcher *> watcherList;

    D_DECLARE_PUBLIC(DBaseFileWatcher)
};

DCORE_END_NAMESPACE

#endif // DBASEFILEWATCHER_P_H
