/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DFILEWATCHERMANAGER_H
#define DFILEWATCHERMANAGER_H

#include "dtkcore_global.h"
#include "base/dobject.h"

#include <QObject>

DCORE_BEGIN_NAMESPACE

class DFileWatcher;

class DFileWatcherManagerPrivate;
class DFileWatcherManager : public QObject, public DObject
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
