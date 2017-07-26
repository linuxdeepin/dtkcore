/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/


#pragma once

#include <QObject>
#include <QScopedPointer>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettingsBackend : public QObject
{
    Q_OBJECT
public:
    explicit DSettingsBackend(QObject *parent = 0): QObject(parent)
    {
        connect(this, &DSettingsBackend::sync, this, &DSettingsBackend::doSync, Qt::QueuedConnection);
        connect(this, &DSettingsBackend::setOption, this, &DSettingsBackend::doSetOption, Qt::QueuedConnection);
    }
    virtual ~DSettingsBackend() {}

    virtual QStringList keys() const = 0;
    virtual QVariant getOption(const QString &key) const = 0;

    virtual void doSync() = 0;

protected:
    virtual void doSetOption(const QString &key, const QVariant &value) = 0;

Q_SIGNALS:
    void sync();
    void setOption(const QString &key, const QVariant &value);
    void optionChanged(const QString &key, const QVariant &value);
};

DCORE_END_NAMESPACE
