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

/*!
  \class Dtk::Core::DSettingsOption
  \inmodule dtkcore
  \brief DSettingsOption is the base key/value item of DSettings.
  \brief DSettingsOption是DSettings的基本单元，用于存放一对键-值数据。
 */

/*!
  \fn void DSettingsOption::valueChanged(QVariant value);
  \brief Emit when option value change.
  \brief 选项的数据变化时发出改信息.

  \a value 发生改变的数据.
 */

/*!
  \fn void DSettingsOption::dataChanged(const QString &dataType, QVariant value);
  \brief Emit when option data change.
  \brief 选项的附件的额外数据变化时发出改信息，可以看作这个值的属性发生变化.

  \a dataType 改变的数据类型, \a value 改变的数据.
 */

/*!
  \property Dtk::Core::DSettingsOption::value
  \brief Current value of this option.
 */

DSettingsOption::DSettingsOption(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsOptionPrivate(this))
{
}

DSettingsOption::~DSettingsOption()
{

}

/*!
  \brief Get direct parent group of this option.
  \brief 当前选项的直接上级组。
  \return 返回当前选项的直接上级组.
 */
QPointer<DSettingsGroup> DSettingsOption::parentGroup() const
{
    Q_D(const DSettingsOption);
    return d->parent;
}

/*!
  \brief Change the direct parent group of this option.
  \brief 修改但前选项的上级组.

  \a parentGroup 上级组.
 */
void DSettingsOption::setParentGroup(QPointer<DSettingsGroup> parentGroup)
{
    Q_D(DSettingsOption);
    d->parent = parentGroup;
}

/*!
  \brief Return the full key of this option, include all parent.
  \brief 当前选项的键值.
  \return 返回当前选项的键值.
 */
QString DSettingsOption::key() const
{
    Q_D(const DSettingsOption);
    return d->key;
}

/*!
  \brief Get display name of the option, it may be translated.
  \brief 当前选项的名称.
  \return 返回当前选项的名称.
 */
QString DSettingsOption::name() const
{
    Q_D(const DSettingsOption);
    return d->name;
}

/*!
  \brief Check this option can be reset to default value. if false, reset action will not take effect.
  \brief 选项是否可以重置，如果可以重置，在调用reset方法后，选项的值会变成初始值.

  \return true if can be reset.
 */
bool DSettingsOption::canReset() const
{
    Q_D(const DSettingsOption);
    return d->canReset;
}

/*!
  \brief Default value of this option, must config in this json desciption file.
  \brief 选项的默认值.

  \return 返回选项的默认值.
 */
QVariant DSettingsOption::defaultValue() const
{
    Q_D(const DSettingsOption);
    return d->defalutValue;
}

/*!
  \brief Get current value of option.
  \brief 选项的当前值.

  \return 返回选项的当前值.
 */
QVariant DSettingsOption::value() const
{
    Q_D(const DSettingsOption);
    return (!d->value.isValid() || d->value.isNull()) ? d->defalutValue : d->value;
}

/*!
  \brief Custom data of option, like QObject::property.
  \a dataType 数据类型.
  \brief 选项的附件data，用于未选项设置一些额外的辅助属性.

  \return 数据类型对应的数据.
  \sa QObject::property
  \sa Dtk::Core::DSettingsOption::setData
 */
QVariant DSettingsOption::data(const QString &dataType) const
{
    Q_D(const DSettingsOption);
    return d->datas.value(dataType);
}

/*!
  \brief UI widget type of this option.
  \brief 选项的控件类型.

  \return 返回选项的控件类型.
  \sa Dtk::Widget::DSettingsWidgetFactory
 */
QString DSettingsOption::viewType() const
{
    Q_D(const DSettingsOption);
    return d->viewType;
}

/*!
  \brief Check this option will show on DSettings dialog.
  \brief 检查选项是否会在界面上显示.

  \return true if option not bind to ui element.
  \return 如果显示则返回true，否则返回false。
 */
bool DSettingsOption::isHidden() const
{
    Q_D(const DSettingsOption);
    return d->hidden;
}

/*!
  \brief Convert QJsonObject to DSettingsOption.
  \brief 从json对象中反序列化出一个选项对象.

  \a prefixKey instead parse prefix key from parent.
  \a prefixKey 选项的前缀
  \a json is an QJsonObejct instance.
  \a json 待反序列化的json对象

  \return 返回解析完成后的 option 数据.
 */
QPointer<DSettingsOption> DSettingsOption::fromJson(const QString &prefixKey, const QJsonObject &json)
{
    auto optionPtr = QPointer<DSettingsOption>(new DSettingsOption);
    optionPtr->parseJson(prefixKey, json);
    return optionPtr;
}

/*!
  \brief Set current value of option.
  \brief 设置选项的当前值.

  \a value 选项的当前值.
 */
void DSettingsOption::setValue(QVariant value)
{
    Q_D(DSettingsOption);

    // 默认没有设置value时比较默认值。防止reset时出现所有的option都发射valueChanged
    if (this->value() == value) {
        return;
    }

    d->value = value;
    Q_EMIT valueChanged(value);
}

///*!
// * \brief Override default value of json
// * \a value
// */
//void DSettingsOption::setDefault(QVariant value)
//{
//    Q_D(DSettingsOption);
//    d->defalutValue = value;
//}

/*!
  \brief Set custom data.
  \brief 为选项添加自定义属性.
  \a dataType is data id, just a unique string.
  \a value of the data id.
  \a dataType 选项的扎属性数据id，对每个选项必须唯一
  \a value 选项id对应的值
  \sa Dtk::Core::DSettingsOption::data
  \sa Dtk::Core::DSettingsOption::data
 */
void DSettingsOption::setData(const QString &dataType, QVariant value)
{
    Q_D(DSettingsOption);

    if (d->datas.value(dataType) == value) {
        return;
    }

    d->datas.insert(dataType, value);

    Q_EMIT dataChanged(dataType, value);
}

/*!
  \brief Parse QJsonObject to DSettingsOption.
  \brief 从json对象中反序列化，并设置自身的值。
  \a prefixKey instead parse prefix key from parent.
  \a json is an QJsonObejct instance.
  \a 选项的前缀
  \a 待反序列化的json对象
  \sa QPointer<DSettingsOption> Dtk::Core::DSettingsOption::fromJson(const QString &prefixKey, const QJsonObject &json)
  \sa QPointer<DSettingsOption> Dtk::Core::DSettingsOption::fromJson(const QString &prefixKey, const QJsonObject &json)
 */
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
            datas.insert(key, value.toVariant());
        }
    }
}

DCORE_END_NAMESPACE


