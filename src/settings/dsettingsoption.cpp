/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dsettingsoption.h"

#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>

DCORE_BEGIN_NAMESPACE

class DSettingsOptionPrivate
{
public:
    DSettingsOptionPrivate(DSettingsOption *parent) : q_ptr(parent) {}

    void parseJson(const QString &prefixKey, const QJsonObject &option);

    QPointer<DSettingsGroup>             parent;

    QString     key;
    QString     name;
    QString     viewType;
    QVariant    defalutValue;
    QVariant    value;
    QVariantMap datas;
    bool        canReset;
    bool        hidden;

    DSettingsOption *q_ptr;
    Q_DECLARE_PUBLIC(DSettingsOption)
};

DSettingsOption::DSettingsOption(QObject *parent) :
    QObject(parent), d_ptr(new DSettingsOptionPrivate(this))
{

}

DSettingsOption::~DSettingsOption()
{

}

QPointer<DSettingsGroup> DSettingsOption::parentGroup() const
{
    Q_D(const DSettingsOption);
    return d->parent;
}

void DSettingsOption::setParentGroup(QPointer<DSettingsGroup> parentGroup)
{
    Q_D(DSettingsOption);
    d->parent = parentGroup;
}

QString DSettingsOption::key() const
{
    Q_D(const DSettingsOption);
    return d->key;
}

QString DSettingsOption::name() const
{
    Q_D(const DSettingsOption);
    return d->name;
}

bool DSettingsOption::canReset() const
{
    Q_D(const DSettingsOption);
    return d->canReset;
}

QVariant DSettingsOption::defaultValue() const
{
    Q_D(const DSettingsOption);
    return d->defalutValue;
}

QVariant DSettingsOption::value() const
{
    Q_D(const DSettingsOption);
    return d->value;
}

QVariant DSettingsOption::data(const QString &dataType) const
{
    Q_D(const DSettingsOption);
    return d->datas.value(dataType);
}

QString DSettingsOption::viewType() const
{
    Q_D(const DSettingsOption);
    return d->viewType;
}

bool DSettingsOption::isHidden() const
{
    Q_D(const DSettingsOption);
    return d->hidden;
}

QPointer<DSettingsOption> DSettingsOption::fromJson(const QString &prefixKey, const QJsonObject &group)
{
    auto optionPtr = QPointer<DSettingsOption>(new DSettingsOption);
    optionPtr->parseJson(prefixKey, group);
    return optionPtr;
}

void DSettingsOption::setValue(QVariant value)
{
    Q_D(DSettingsOption);

    if (d->value == value) {
        return;
    }

    d->value = value;
    Q_EMIT valueChanged(value);
}

void DSettingsOption::setData(const QString &dataType, QVariant value)
{
    Q_D(DSettingsOption);

    if (d->datas.value(dataType) == value) {
        return;
    }

    d->datas.insert(dataType, value);

    Q_EMIT dataChanged(dataType, value);
}

void DSettingsOption::parseJson(const QString &prefixKey, const QJsonObject &option)
{
    Q_D(DSettingsOption);
    d->parseJson(prefixKey, option);
}

void DSettingsOptionPrivate::parseJson(const QString &prefixKey, const QJsonObject &option)
{
//    Q_Q(Option);
    key = option.value("key").toString();
    Q_ASSERT(!key.isEmpty());
    Q_ASSERT(!prefixKey.isEmpty());
    key = prefixKey + "." + key;
    name = option.value("name").toString();

    canReset = !option.contains("reset") ? true : option.value("reset").toBool();
    defalutValue = option.value("default").toVariant();
    hidden = !option.contains("hide") ? false : option.value("hide").toBool();
    viewType = option.value("type").toString();
    value = defalutValue;

    QStringList revserdKeys;
    revserdKeys << "key" << "name" << "reset"
                << "default" << "hide" << "type";

    auto allKeys = option.keys();
    for (auto key : revserdKeys) {
        allKeys.removeAll(key);
    }

    for (auto key : allKeys) {
        auto value = option.value(key);
        if (value.isArray()) {
            QStringList stringlist;
            for (auto va : value.toArray()) {
                stringlist << QString("%1").arg(va.toString());
            }
            datas.insert(key, stringlist);
        } else {
            datas.insert(key, value);
        }
    }
}

DCORE_END_NAMESPACE


