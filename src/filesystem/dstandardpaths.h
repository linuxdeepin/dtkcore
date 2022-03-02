/*
 * Copyright (C) 2017 ~ 2021 Deepin Technology Co., Ltd.
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

#ifndef DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H
#define DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H

#include <QStandardPaths>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DStandardPathsPrivate;
class LIBDTKCORESHARED_EXPORT DStandardPaths
{
public:
    enum Mode {
        Auto,
        Snap,
        Test,
    };

    static QString writableLocation(QStandardPaths::StandardLocation type);
    static QStringList standardLocations(QStandardPaths::StandardLocation type);

    static QString locate(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options = QStandardPaths::LocateFile);
    static QStringList locateAll(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options = QStandardPaths::LocateFile);
    static QString findExecutable(const QString &executableName, const QStringList &paths = QStringList());
    static void setMode(Mode mode);

    enum class XDG {
        DataHome,
        ConfigHome,
        CacheHome,
        RuntimeTime
    };

    enum class DSG {
        AppData,
        DataDir
    };

    static QString homePath();
    static QString homePath(const uint uid);
    static QString path(XDG type);
    static QString path(DSG type);
    static QStringList paths(DSG type);
    static QString filePath(XDG type, QString fileName);
    static QString filePath(DSG type, QString fileName);

private:
    DStandardPaths();
    ~DStandardPaths();
    Q_DISABLE_COPY(DStandardPaths)
};

DCORE_END_NAMESPACE

#endif // DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H
