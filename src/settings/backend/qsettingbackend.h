/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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

#include <dsettingsbackend.h>

DCORE_BEGIN_NAMESPACE

class QSettingBackendPrivate;
class LIBDTKCORESHARED_EXPORT QSettingBackend : public Dtk::Core::DSettingsBackend
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
