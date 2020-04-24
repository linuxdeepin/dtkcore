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

#ifndef DFILESYSTEMWATCHER_P_H
#define DFILESYSTEMWATCHER_P_H

#include "base/private/dobject_p.h"

#include <QSocketNotifier>
#include <QHash>
#include <QMap>

DCORE_BEGIN_NAMESPACE

class DFileSystemWatcher;
class DFileSystemWatcherPrivate : public DObjectPrivate
{
    Q_DECLARE_PUBLIC(DFileSystemWatcher)

public:
    DFileSystemWatcherPrivate(int fd, DFileSystemWatcher *qq);
    ~DFileSystemWatcherPrivate();

    QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories);
    QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories);

    QStringList files, directories;
    int inotifyFd;
    QHash<QString, int> pathToID;
    QMultiHash<int, QString> idToPath;
    QSocketNotifier notifier;

    // private slots
    void _q_readFromInotify();

private:
    void onFileChanged(const QString &path, bool removed);
    void onDirectoryChanged(const QString &path, bool removed);
};

DCORE_END_NAMESPACE

#endif // DFILESYSTEMWATCHER_P_H
