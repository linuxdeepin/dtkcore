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
#include <QPointer>

#include "dsettingsoption.h"

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettingsGroupPrivate;
class LIBDTKCORESHARED_EXPORT DSettingsGroup : public QObject
{
    Q_OBJECT
public:
    explicit DSettingsGroup(QObject *parent = Q_NULLPTR);
    ~DSettingsGroup();

    QPointer<DSettingsGroup> parentGroup() const;
    void setParentGroup(QPointer<DSettingsGroup> parentGroup);

    QString key() const;
    QString name() const;
    bool isHidden() const;

    QPointer<DSettingsGroup> childGroup(const QString &groupKey) const;
    QPointer<DSettingsOption> option(const QString &key) const;

    QList<QPointer<DSettingsGroup> > childGroups() const;
    QList<QPointer<DSettingsOption> > childOptions() const;
    QList<QPointer<DSettingsOption> > options() const;

    static QPointer<DSettingsGroup> fromJson(const QString &prefixKey, const QJsonObject &group);

private:
    void parseJson(const QString &prefixKey, const QJsonObject &group);

    QScopedPointer<DSettingsGroupPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), DSettingsGroup)
};

typedef QPointer<DSettingsGroup> GroupPtr;

DCORE_END_NAMESPACE
