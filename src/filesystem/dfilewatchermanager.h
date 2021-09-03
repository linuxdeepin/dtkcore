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

#ifndef DFILEWATCHERMANAGER_H
#define DFILEWATCHERMANAGER_H

#include <dtkcore_global.h>
#include <DObject>

#include <QObject>

DCORE_BEGIN_NAMESPACE

class DFileWatcher;

class DFileWatcherManagerPrivate;
class LIBDTKCORESHARED_EXPORT DFileWatcherManager : public QObject, public DObject
{
    Q_OBJECT

public:
    explicit DFileWatcherManager(QObject *parent = 0);
    ~DFileWatcherManager();

    DFileWatcher *add(const QString &filePath);
    void remove(const QString &filePath);

Q_SIGNALS:
    void fileDeleted(const QString &filePath);
    void fileAttributeChanged(const QString &filePath);
    void fileMoved(const QString &fromFilePath, const QString &toFilePath);
    void subfileCreated(const QString &filePath);
    void fileModified(const QString &filePath);
    void fileClosed(const QString &filePath);

private:
    QScopedPointer<DFileWatcherManagerPrivate> d_ptr;

    D_DECLARE_PRIVATE(DFileWatcherManager)
    Q_DISABLE_COPY(DFileWatcherManager)
};

DCORE_END_NAMESPACE

#endif // DFILEWATCHERMANAGER_H
