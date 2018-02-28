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

#include "dsettingsgroup.h"

#include <QMap>
#include <QJsonObject>
#include <QJsonArray>

DCORE_BEGIN_NAMESPACE

class DSettingsGroupPrivate
{
public:
    DSettingsGroupPrivate(DSettingsGroup *parent) : q_ptr(parent) {}

    QString key;
    QString name;

    QMap<QString, OptionPtr>    options;

    QPointer<DSettingsGroup>             parent;
    QMap<QString, OptionPtr>    childOptions;
    QList<QString>              childOptionKeys;

    QMap<QString, GroupPtr>     childGroups;
    QList<QString>              childGroupKeys;

    void parseJson(const QString &prefixKey, const QJsonObject &group);

    DSettingsGroup *q_ptr;
    Q_DECLARE_PUBLIC(DSettingsGroup)
};

DSettingsGroup::DSettingsGroup(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsGroupPrivate(this))
{

}

DSettingsGroup::~DSettingsGroup()
{

}

QPointer<DSettingsGroup> DSettingsGroup::parentGroup() const
{
    Q_D(const DSettingsGroup);
    return d->parent;
}

void DSettingsGroup::setParentGroup(QPointer<DSettingsGroup> parentGroup)
{
    Q_D(DSettingsGroup);
    d->parent = parentGroup;
}

QString DSettingsGroup::key() const
{
    Q_D(const DSettingsGroup);
    return d->key;
}

QString DSettingsGroup::name() const
{
    Q_D(const DSettingsGroup);
    return d->name;
}

QPointer<DSettingsGroup> DSettingsGroup::childGroup(const QString &key) const
{
    Q_D(const DSettingsGroup);
    return d->childGroups.value(key);
}

QList<QPointer<DSettingsGroup> > DSettingsGroup::childGroups() const
{
    Q_D(const DSettingsGroup);
    QList<QPointer<DSettingsGroup> > grouplist;
    for (auto groupKey : d->childGroupKeys) {
        grouplist << d->childGroups.value(groupKey);
    }
    return grouplist;
}

QList<QPointer<DSettingsOption> > DSettingsGroup::childOptions() const
{
    Q_D(const DSettingsGroup);
    QList<QPointer<DSettingsOption> > optionlist;
    for (auto optionKey : d->childOptionKeys) {
        optionlist << d->childOptions.value(optionKey);
    }
    return optionlist;
}

QList<QPointer<DSettingsOption> > DSettingsGroup::options() const
{
    Q_D(const DSettingsGroup);
    return d->options.values();
}

QPointer<DSettingsGroup> DSettingsGroup::fromJson(const QString &prefixKey, const QJsonObject &group)
{
    auto groupPtr = QPointer<DSettingsGroup>(new DSettingsGroup);
    groupPtr->parseJson(prefixKey, group);
    return groupPtr;
}

void DSettingsGroup::parseJson(const QString &prefixKey, const QJsonObject &group)
{
    Q_D(DSettingsGroup);
    d->parseJson(prefixKey, group);
}

void DSettingsGroupPrivate::parseJson(const QString &prefixKey, const QJsonObject &group)
{
    Q_Q(DSettingsGroup);
    key = group.value("key").toString();
    Q_ASSERT(!key.isEmpty());
    key = prefixKey.isEmpty() ? key : prefixKey + "." + key;
    name = group.value("name").toString();

    for (auto optionJson :  group.value("options").toArray()) {
        auto optionObject = optionJson.toObject();
        auto option = DSettingsOption::fromJson(key, optionObject);
        options.insert(option->key(), option);
        childOptions.insert(option->key(), option);
        childOptionKeys << option->key();
    }

    auto subGroups = group.value("groups").toArray();
    for (auto subGroup : subGroups) {
        auto child = DSettingsGroup::fromJson(key, subGroup.toObject());
        child->setParentGroup(q);
        childGroups.insert(child->key(), child);
        childGroupKeys << child->key();

        for (auto option : child->options()) {
            options.insert(option->key(), option);
        }
    }

}

DCORE_END_NAMESPACE
