/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H
#define DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H

#include <QStandardPaths>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DStandardPathsPrivate;
class DStandardPaths
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

private:
    DStandardPaths();
    ~DStandardPaths();
    Q_DISABLE_COPY(DStandardPaths)
};

DCORE_END_NAMESPACE

#endif // DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H
