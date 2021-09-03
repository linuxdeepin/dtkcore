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

#include <QVariant>
#include <QObject>
#include <QPointer>

#include <dtkcore_global.h>

DCORE_BEGIN_NAMESPACE

class DSettingsGroup;
class DSettingsOptionPrivate;
class LIBDTKCORESHARED_EXPORT DSettingsOption : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit DSettingsOption(QObject *parent = Q_NULLPTR);
    ~DSettingsOption();

    QPointer<DSettingsGroup> parentGroup() const;
    void setParentGroup(QPointer<DSettingsGroup> parentGroup);

    QString key() const;
    QString name() const;
    bool canReset() const;
    QVariant defaultValue() const;
    QVariant value() const;
    QVariant data(const QString &dataType) const;

    QString viewType() const;
    bool isHidden() const;

    static QPointer<DSettingsOption> fromJson(const QString &prefixKey, const QJsonObject &json);
Q_SIGNALS:
    void valueChanged(QVariant value);
    void dataChanged(const QString &dataType, QVariant value);

public Q_SLOTS:
    void setValue(QVariant value);
    void setData(const QString &dataType, QVariant value);

private:
    void parseJson(const QString &prefixKey, const QJsonObject &option);

    QScopedPointer<DSettingsOptionPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), DSettingsOption)
};

typedef QPointer<DSettingsOption> OptionPtr;

DCORE_END_NAMESPACE
