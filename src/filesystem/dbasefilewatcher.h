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

#ifndef DBASEFILEWATCHER_H
#define DBASEFILEWATCHER_H

#include <dtkcore_global.h>
#include <DObject>

#include <QObject>

DCORE_BEGIN_NAMESPACE

class DBaseFileWatcherPrivate;
class LIBDTKCORESHARED_EXPORT DBaseFileWatcher : public QObject, public DObject
{
    Q_OBJECT

public:
    ~DBaseFileWatcher();

    QUrl fileUrl() const;

    bool startWatcher();
    bool stopWatcher();
    bool restartWatcher();

    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true);

    using SignalType1 = void(DBaseFileWatcher::*)(const QUrl &);
    using SignalType2 = void(DBaseFileWatcher::*)(const QUrl &, const QUrl &);
    static bool ghostSignal(const QUrl &targetUrl, SignalType1 signal, const QUrl &arg1);
    static bool ghostSignal(const QUrl &targetUrl, SignalType2 signal, const QUrl &arg1, const QUrl &arg2);

Q_SIGNALS:
    void fileDeleted(const QUrl &url);
    void fileAttributeChanged(const QUrl &url);
    void fileMoved(const QUrl &fromUrl, const QUrl &toUrl);
    void subfileCreated(const QUrl &url);
    void fileModified(const QUrl &url);
    void fileClosed(const QUrl &url);

protected:
    explicit DBaseFileWatcher(DBaseFileWatcherPrivate &dd, const QUrl &url, QObject *parent = 0);

private:
    Q_DISABLE_COPY(DBaseFileWatcher)
    D_DECLARE_PRIVATE(DBaseFileWatcher)
};

DCORE_END_NAMESPACE

#endif // DBASEFILEWATCHER_H
