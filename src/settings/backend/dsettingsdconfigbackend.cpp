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

#include "dsettingsdconfigbackend.h"

#include <QDebug>
#include <QMutex>
#include <DConfig>

DCORE_BEGIN_NAMESPACE

class DSettingsDConfigBackendPrivate
{
public:
    explicit DSettingsDConfigBackendPrivate(DSettingsDConfigBackend *parent) : q_ptr(parent) {}

    DConfig       *dConfig   = nullptr;
    QMutex          writeLock;

    DSettingsDConfigBackend *q_ptr;
    Q_DECLARE_PUBLIC(DSettingsDConfigBackend)
};

/*!
  \class Dtk::Core::DSettingsDConfigBackend
  \inmodule dtkcore
  \brief Storage DSetttings to an DConfig.
 */

/*!
  \brief Save data to configure file name with DConfig.
  \a name configure file name.
  \a subpath subdirectory of configure file name.
  \a parent
 */
DSettingsDConfigBackend::DSettingsDConfigBackend(const QString &name, const QString &subpath, QObject *parent) :
    DSettingsBackend(parent), d_ptr(new DSettingsDConfigBackendPrivate(this))
{
    Q_D(DSettingsDConfigBackend);

    d->dConfig = new DConfig(name, subpath, this);
}

DSettingsDConfigBackend::~DSettingsDConfigBackend()
{

}

/*!
  \brief List all keys of DConfig
  \return
 */
QStringList DSettingsDConfigBackend::keys() const
{
    Q_D(const DSettingsDConfigBackend);
    return d->dConfig->keyList();
}

/*!
  \brief Get value of key from DConfig
  \a key
  \return
 */
QVariant DSettingsDConfigBackend::getOption(const QString &key) const
{
    Q_D(const DSettingsDConfigBackend);
    return d->dConfig->value(key);
}

/*!
  \brief Set value of key to DConfig
  \a key
  \a value
 */
void DSettingsDConfigBackend::doSetOption(const QString &key, const QVariant &value)
{
    Q_D(DSettingsDConfigBackend);
    d->writeLock.lock();
    d->dConfig->setValue(key, value);
    d->writeLock.unlock();
}

/*!
  \brief Trigger DSettings to save option value to DConfig
 */
void DSettingsDConfigBackend::doSync()
{
    Q_D(DSettingsDConfigBackend);

    // TODO
}


DCORE_END_NAMESPACE
