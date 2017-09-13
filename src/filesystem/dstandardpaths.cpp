/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include "dstandardpaths.h"

#include <QProcessEnvironment>

DCORE_BEGIN_NAMESPACE


class DSnapStandardPaths
{
public:
    inline  static QString writableLocation(QStandardPaths::StandardLocation /*type*/)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        return env.value("SNAP_USER_COMMON");
    }

    inline static QStringList standardLocations(QStandardPaths::StandardLocation type)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

        switch (type) {
        case QStandardPaths::GenericDataLocation: {
            QString snapRoot = env.value("SNAP");
            QString genericDataDir = snapRoot + "/usr/share/";
            return QStringList() << genericDataDir;
        }
        default:
            break;
        }

        return QStringList() << env.value("SNAP_USER_COMMON");
    }

private:
    DSnapStandardPaths();
    ~DSnapStandardPaths();
    Q_DISABLE_COPY(DSnapStandardPaths)
};


static DStandardPaths::Mode s_mode = DStandardPaths::Auto;

QString DStandardPaths::writableLocation(QStandardPaths::StandardLocation type)
{
    switch (s_mode) {
    case Auto:
    case Test:
        return  QStandardPaths::writableLocation(type);
    case Snap:
        return DSnapStandardPaths::writableLocation(type);
    }
    return QStandardPaths::writableLocation(type);
}

QStringList DStandardPaths::standardLocations(QStandardPaths::StandardLocation type)
{
    switch (s_mode) {
    case Auto:
    case Test:
        return  QStandardPaths::standardLocations(type);
    case Snap:
        return DSnapStandardPaths::standardLocations(type);
    }
    return  QStandardPaths::standardLocations(type);
}

QString DStandardPaths::locate(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options)
{
    return QStandardPaths::locate(type, fileName, options);
}

QStringList DStandardPaths::locateAll(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options)
{
    return QStandardPaths::locateAll(type, fileName, options);
}

QString DStandardPaths::findExecutable(const QString &executableName, const QStringList &paths)
{
    return QStandardPaths::findExecutable(executableName, paths);
}

void DStandardPaths::setMode(DStandardPaths::Mode mode)
{
    s_mode = mode;
    QStandardPaths::setTestModeEnabled(mode == Test);
}

DCORE_END_NAMESPACE
