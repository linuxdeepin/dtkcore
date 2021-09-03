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

#ifndef DFILESYSTEMWATCHER_H
#define DFILESYSTEMWATCHER_H

#include <dtkcore_global.h>
#include <DObject>

#include <QObject>

DCORE_BEGIN_NAMESPACE

class DFileSystemWatcherPrivate;
class LIBDTKCORESHARED_EXPORT DFileSystemWatcher : public QObject, public DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DFileSystemWatcher)

public:
    DFileSystemWatcher(QObject *parent = Q_NULLPTR);
    DFileSystemWatcher(const QStringList &paths, QObject *parent = Q_NULLPTR);
    ~DFileSystemWatcher();

    bool addPath(const QString &file);
    QStringList addPaths(const QStringList &files);
    bool removePath(const QString &file);
    QStringList removePaths(const QStringList &files);

    QStringList files() const;
    QStringList directories() const;

Q_SIGNALS:
    void fileDeleted(const QString &path, const QString &name, QPrivateSignal);
    void fileAttributeChanged(const QString &path, const QString &name, QPrivateSignal);
    void fileClosed(const QString &path, const QString &name, QPrivateSignal);
    void fileMoved(const QString &fromPath, const QString &fromName,
                   const QString &toPath, const QString &toName, QPrivateSignal);
    void fileCreated(const QString &path, const QString &name, QPrivateSignal);
    void fileModified(const QString &path, const QString &name, QPrivateSignal);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_readFromInotify())
};

DCORE_END_NAMESPACE

#endif // DFILESYSTEMWATCHER_H
