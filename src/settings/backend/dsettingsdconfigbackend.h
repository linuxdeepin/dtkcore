/*
 * Copyright (C) 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Wang Fei <wangfeia@uniontech.com>
 *
 * Maintainer: Wang Fei <wangfeia@uniontech.com>
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

#pragma once

#include <QObject>
#include <QScopedPointer>

#include "dsettingsbackend.h"

DCORE_BEGIN_NAMESPACE

class DSettingsDConfigBackendPrivate;
class LIBDTKCORESHARED_EXPORT DSettingsDConfigBackend : public Dtk::Core::DSettingsBackend
{
    Q_OBJECT
public:
    explicit DSettingsDConfigBackend(const QString &name, const QString &subpath = QString(), QObject *parent = nullptr);
    ~DSettingsDConfigBackend() Q_DECL_OVERRIDE;

    virtual QStringList keys() const Q_DECL_OVERRIDE;
    virtual QVariant getOption(const QString &key) const Q_DECL_OVERRIDE;

protected Q_SLOTS:
    virtual void doSetOption(const QString &key, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void doSync() Q_DECL_OVERRIDE;

private:
    QScopedPointer<DSettingsDConfigBackendPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettingsDConfigBackend)
};

DCORE_END_NAMESPACE
