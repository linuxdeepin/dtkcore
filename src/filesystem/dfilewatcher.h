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

#ifndef DFILEWATCHER_H
#define DFILEWATCHER_H

#include "dbasefilewatcher.h"

DCORE_BEGIN_NAMESPACE

class DFileWatcherPrivate;
class LIBDTKCORESHARED_EXPORT DFileWatcher : public DBaseFileWatcher
{
    Q_OBJECT

public:
    explicit DFileWatcher(const QString &filePath, QObject *parent = 0);

private Q_SLOTS:
    void onFileDeleted(const QString &path, const QString &name);
    void onFileAttributeChanged(const QString &path, const QString &name);
    void onFileMoved(const QString &fromPath, const QString &fromName,
                     const QString &toPath, const QString &toName);
    void onFileCreated(const QString &path, const QString &name);
    void onFileModified(const QString &path, const QString &name);
    void onFileClosed(const QString &path, const QString &name);

private:
    D_DECLARE_PRIVATE(DFileWatcher)
};

DCORE_END_NAMESPACE

#endif // DFILEWATCHER_H
