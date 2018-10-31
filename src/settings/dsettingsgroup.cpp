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
    bool    hide = false;

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

/*!
 * \class DSettingsGroup
 * \brief A group of DSettingsOption and DSettingsGroup.
 * DSettingsGroup can contain a lost option and subgroup.
 */

DSettingsGroup::DSettingsGroup(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsGroupPrivate(this))
{

}

DSettingsGroup::~DSettingsGroup()
{

}
/*!
 * \brief Get direct parent group of this group.
 * \return
 */
QPointer<DSettingsGroup> DSettingsGroup::parentGroup() const
{
    Q_D(const DSettingsGroup);
    return d->parent;
}

/*!
 * \brief Change the direct parent group of this group.
 * \param parentGroup
 */
void DSettingsGroup::setParentGroup(QPointer<DSettingsGroup> parentGroup)
{
    Q_D(DSettingsGroup);
    d->parent = parentGroup;
}

/*!
 * \brief Return the full key of this group, include all parent.
 * \return
 */
QString DSettingsGroup::key() const
{
    Q_D(const DSettingsGroup);
    return d->key;
}

/*!
 * \brief Get display name of this group, it may be translated.
 * \return
 */
QString DSettingsGroup::name() const
{
    Q_D(const DSettingsGroup);
    return d->name;
}

/*!
 * \brief Check this group will show on DSettings dialog.
 * \return true if group not bind to ui element.
 */
bool DSettingsGroup::isHidden() const
{
    Q_D(const DSettingsGroup);
    return d->hide;
}

/*!
 * \brief Get the child group of groupKey
 * \param groupKey is child group key
 * \return
 */
QPointer<DSettingsGroup> DSettingsGroup::childGroup(const QString &groupKey) const
{
    Q_D(const DSettingsGroup);
    return d->childGroups.value(groupKey);
}

/*!
 * \brief Get the child option of key
 * \param key is child option key
 * \return
 */
QPointer<DSettingsOption> DSettingsGroup::option(const QString &key) const
{
    Q_D(const DSettingsGroup);
    return d->childOptions.value(key);
}

/*!
 * \brief Enum all direct child group of this group
 * \return
 */
QList<QPointer<DSettingsGroup> > DSettingsGroup::childGroups() const
{
    Q_D(const DSettingsGroup);
    QList<QPointer<DSettingsGroup> > grouplist;
    for (auto groupKey : d->childGroupKeys) {
        grouplist << d->childGroups.value(groupKey);
    }
    return grouplist;
}

/*!
 * \brief Enum all direct child option with the raw order in json description file.
 * \return
 */
QList<QPointer<DSettingsOption> > DSettingsGroup::childOptions() const
{
    Q_D(const DSettingsGroup);
    QList<QPointer<DSettingsOption> > optionlist;
    for (auto optionKey : d->childOptionKeys) {
        optionlist << d->childOptions.value(optionKey);
    }
    return optionlist;
}

/*!
 * \brief Enum all direct child option of this group.
 * \return
 */
QList<QPointer<DSettingsOption> > DSettingsGroup::options() const
{
    Q_D(const DSettingsGroup);
    return d->options.values();
}

/*!
 * \brief Convert QJsonObject to DSettingsGroup.
 * \param prefixKey instead parse prefix key from parent.
 * \param json is an QJsonObejct instance.
 * \sa QPointer<DSettingsOption> Dtk::Core::DSettingsGroup::parseJson(const QString &prefixKey, const QJsonObject &json)
 */
QPointer<DSettingsGroup> DSettingsGroup::fromJson(const QString &prefixKey, const QJsonObject &group)
{
    auto groupPtr = QPointer<DSettingsGroup>(new DSettingsGroup);
    groupPtr->parseJson(prefixKey, group);
    return groupPtr;
}

/*!
 * \brief Parse QJsonObject to DSettingsGroup.
 * \param prefixKey instead parse prefix key from parent.
 * \param json is an QJsonObejct instance.
 * \sa QPointer<DSettingsOption> Dtk::Core::DSettingsGroup::fromJson(const QString &prefixKey, const QJsonObject &json)
 */
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
    hide = group.value("hide").toBool();

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
