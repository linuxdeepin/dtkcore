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

#include "dsettingsbackend.h"

DCORE_BEGIN_NAMESPACE

class QSettingBackendPrivate;
class QSettingBackend : public Dtk::Core::DSettingsBackend
{
    Q_OBJECT
public:
    explicit QSettingBackend(const QString &filepath, QObject *parent = 0);
    ~QSettingBackend();

    virtual QStringList keys() const Q_DECL_OVERRIDE;
    virtual QVariant getOption(const QString &key) const Q_DECL_OVERRIDE;

protected Q_SLOTS:
    virtual void doSetOption(const QString &key, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void doSync() Q_DECL_OVERRIDE;

private:
    QScopedPointer<QSettingBackendPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), QSettingBackend)
};

DCORE_END_NAMESPACE
