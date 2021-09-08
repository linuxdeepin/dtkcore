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
  \class Dtk::Core::DSettingsGroup
  \inmodule dtkcore
  \brief A group of DSettingsOption and DSettingsGroup.
  DSettingsGroup can contain a lost option and subgroup.
  \brief 一组DSettings选项的集合，也可以包含子组。
 */


DSettingsGroup::DSettingsGroup(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsGroupPrivate(this))
{

}

DSettingsGroup::~DSettingsGroup()
{

}

/*!
  \brief Get direct parent group of this group.
  \brief 获取当前组的父组。
  \return
 */
QPointer<DSettingsGroup> DSettingsGroup::parentGroup() const
{
    Q_D(const DSettingsGroup);
    return d->parent;
}

/*!
  \brief Change the direct \a parentGroup of this group.
  \brief 设置当前组的父组为 \a parentGroup 。
 */
void DSettingsGroup::setParentGroup(QPointer<DSettingsGroup> parentGroup)
{
    Q_D(DSettingsGroup);
    d->parent = parentGroup;
}

/*!
  \brief Return the full key of this group, include all parent.
  \return 返回这个组的键，会包含全部的父组的键。
 */
QString DSettingsGroup::key() const
{
    Q_D(const DSettingsGroup);
    return d->key;
}

/*!
  \brief Get display name of this group, it may be translated.
  \return 返回这个组名称。
 */
QString DSettingsGroup::name() const
{
    Q_D(const DSettingsGroup);
    return d->name;
}

/*!
  \brief Check this group will show on DSettings dialog.
  \brief 检查这个选项组是否会在界面上显示。
  \return true 表示则这个选项组会显示出来。
 */
bool DSettingsGroup::isHidden() const
{
    Q_D(const DSettingsGroup);
    return d->hide;
}

/*!
  \brief Get the child group of groupKey
  \a groupKey is child group key
  \brief 返回给定键在选项组中对应的子组。
  \a groupKey 子组的键

  \return 返回子组的指针.
 */
QPointer<DSettingsGroup> DSettingsGroup::childGroup(const QString &groupKey) const
{
    Q_D(const DSettingsGroup);
    return d->childGroups.value(groupKey);
}

/*!
  \brief Get the child option of key
  \a key is child option key
  \brief 根据键值获取选项。
  \a key 选项的完整键

  \return 返回对应键值选项指针.
 */
QPointer<DSettingsOption> DSettingsGroup::option(const QString &key) const
{
    Q_D(const DSettingsGroup);
    return d->childOptions.value(key);
}

/*!
  \brief Enum all direct child group of this group
  \brief 列出组下面所有的直接子组。

  \return 返回所有子组指针列表.
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
  \brief Enum all direct child option with the raw order in json description file.
  \brief 列出组下面所有的直接选项。
  \return 返回所有子选项指针列表.
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
  \brief Enum all direct child option of this group.
  \brief 列出组下面所有的选项。
  \return 返回所有选项指针列表.
 */
QList<QPointer<DSettingsOption> > DSettingsGroup::options() const
{
    Q_D(const DSettingsGroup);
    return d->options.values();
}

/*!
  \brief Convert QJsonObject to DSettingsGroup.
  \a prefixKey instead parse prefix key from parent.
  \a group is an QJsonObejct instance.
  \brief 将json对象转化为DSettingsGroup
  \a prefixKey 组键值前缀
  \a group 待反序列化的json对象
  \return 返回解析json后的组指针.

  \sa QPointer Dtk::Core::DSettingsOption
 */
QPointer<DSettingsGroup> DSettingsGroup::fromJson(const QString &prefixKey, const QJsonObject &group)
{
    auto groupPtr = QPointer<DSettingsGroup>(new DSettingsGroup);
    groupPtr->parseJson(prefixKey, group);
    return groupPtr;
}

/*!
  \brief Parse QJsonObject to DSettingsGroup.
  \a prefixKey instead parse prefix key from parent.
  \a json is an QJsonObejct instance.
  \sa QPointer<DSettingsOption> Dtk::Core::DSettingsGroup::fromJson(const QString &prefixKey, const QJsonObject &json)
  \brief 将json对象转化为DSettingsGroup
  \a prefixKey 组键值前缀
  \a group 待反序列化的json对象
  \sa QPointer<DSettingsOption> Dtk::Core::DSettingsGroup::fromJson(const QString &prefixKey, const QJsonObject &json)
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
        option->setParent(q);
        options.insert(option->key(), option);
        childOptions.insert(option->key(), option);
        childOptionKeys << option->key();
    }

    auto subGroups = group.value("groups").toArray();
    for (auto subGroup : subGroups) {
        auto child = DSettingsGroup::fromJson(key, subGroup.toObject());
        child->setParent(q);
        child->setParentGroup(q);
        childGroups.insert(child->key(), child);
        childGroupKeys << child->key();

        for (auto option : child->options()) {
            options.insert(option->key(), option);
        }
    }

}

DCORE_END_NAMESPACE
