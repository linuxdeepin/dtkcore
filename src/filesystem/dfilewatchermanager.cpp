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

#include "dfilewatchermanager.h"
#include "dfilewatcher.h"
#include "base/private/dobject_p.h"

#include <QMap>
#include <QUrl>

DCORE_BEGIN_NAMESPACE

class DFileWatcherManagerPrivate : public DObjectPrivate
{
public:
    DFileWatcherManagerPrivate(DFileWatcherManager *qq);

    QMap<QString, DFileWatcher *> watchersMap;

    D_DECLARE_PUBLIC(DFileWatcherManager)
};

DFileWatcherManagerPrivate::DFileWatcherManagerPrivate(DFileWatcherManager *qq)
    : DObjectPrivate(qq)
{

}

/*!
    \class Dtk::Core::DFileWatcherManager
    \inmodule dtkcore
    \brief The DFileWatcherManager class can help you manage file watchers and get signal when file got changed.
    \brief DFileWatcherManager 类可以帮助管理一系列 DFileWatcher 文件监视器，并在文件变动时发送信号通知.
*/

DFileWatcherManager::DFileWatcherManager(QObject *parent)
    : QObject(parent)
    , DObject(*new DFileWatcherManagerPrivate(this))
{

}

DFileWatcherManager::~DFileWatcherManager()
{

}

/*!
  \brief 为路径 \a filePath 创建 DFileWatcher 并将其添加到 DFileWatcherManager 中.
  \brief Add file watcher for \a filePath to the file watcher manager.
  
  \return 被创建并添加到 DFileWatcherManager 的 DFileWatcher
  \return The file watcher which got created and added into the file watcher manager.
 */
DFileWatcher *DFileWatcherManager::add(const QString &filePath)
{
    Q_D(DFileWatcherManager);

    DFileWatcher *watcher = d->watchersMap.value(filePath);

    if (watcher) {
        return watcher;
    }

    watcher = new DFileWatcher(filePath, this);

    connect(watcher, &DFileWatcher::fileAttributeChanged, this, [this](const QUrl & url) {
        Q_EMIT fileAttributeChanged(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileClosed, this, [this](const QUrl & url) {
        Q_EMIT fileClosed(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileDeleted, this, [this](const QUrl & url) {
        Q_EMIT fileDeleted(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileModified, this, [this](const QUrl & url) {
        Q_EMIT fileModified(url.toLocalFile());
    });
    connect(watcher, &DFileWatcher::fileMoved, this, [this](const QUrl & fromUrl, const QUrl & toUrl) {
        Q_EMIT fileMoved(fromUrl.toLocalFile(), toUrl.toLocalFile());
    });
    connect(watcher, &DFileWatcher::subfileCreated, this, [this](const QUrl & url) {
        Q_EMIT subfileCreated(url.toLocalFile());
    });

    d->watchersMap[filePath] = watcher;
    watcher->startWatcher();

    return watcher;
}

/*!
  \brief 从当前 DFileWatcherManager 中移除监视 \a filePath 的 DFileWatcher.
  \brief Remove file watcher for \a filePath from the file watcher manager.
 */
void DFileWatcherManager::remove(const QString &filePath)
{
    Q_D(DFileWatcherManager);

    DFileWatcher *watcher = d->watchersMap.take(filePath);

    if (watcher) {
        watcher->deleteLater();
    }
}

DCORE_END_NAMESPACE
