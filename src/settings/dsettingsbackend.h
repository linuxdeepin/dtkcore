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

#include <dtkcore_global.h>

DCORE_BEGIN_NAMESPACE

class DSettings;
class LIBDTKCORESHARED_EXPORT DSettingsBackend : public QObject
{
    Q_OBJECT
public:
    explicit DSettingsBackend(QObject *parent = Q_NULLPTR): QObject(parent)
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
    void optionChanged(const QString &key, const QVariant &value);

    // private signals;
Q_SIGNALS:
    void sync();
    void setOption(const QString &key, const QVariant &value);
};

DCORE_END_NAMESPACE
