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

/*!
 * \class DSettingsOption
 * \brief DSettingsOption is the base key/value item of DSettings.
 *
 * \fn void DSettingsOption::valueChanged(QVariant value);
 * \brief Emit when option value change.
 *
 * \fn void DSettingsOption::dataChanged(const QString &dataType, QVariant value);
 * \brief Emit when option data change.
 *
 *

 * \property DSettingsOption::value
 * \brief Current value of this option.
 */

DSettingsOption::DSettingsOption(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsOptionPrivate(this))
{
}

DSettingsOption::~DSettingsOption()
{

}

/*!
 * \brief Get direct parent group of this option.
 * \return
 */
QPointer<DSettingsGroup> DSettingsOption::parentGroup() const
{
    Q_D(const DSettingsOption);
    return d->parent;
}

/*!
 * \brief Change the direct parent group of this option.
 * \param parentGroup
 */
void DSettingsOption::setParentGroup(QPointer<DSettingsGroup> parentGroup)
{
    Q_D(DSettingsOption);
    d->parent = parentGroup;
}

/*!
 * \brief Return the full key of this option, include all parent.
 * \return
 */
QString DSettingsOption::key() const
{
    Q_D(const DSettingsOption);
    return d->key;
}

/*!
 * \brief Get display name of the option, it may be translated.
 * \return
 */
QString DSettingsOption::name() const
{
    Q_D(const DSettingsOption);
    return d->name;
}

/*!
 * \brief Check this option can be reset to default value. if false, reset action will not take effect.
 * \return true if can be reset.
 * \sa Dtk::Core::DSettings::reset
 */
bool DSettingsOption::canReset() const
{
    Q_D(const DSettingsOption);
    return d->canReset;
}

/*!
 * \brief Default value of this option, must config in this json desciption file.
 * \return
 */
QVariant DSettingsOption::defaultValue() const
{
    Q_D(const DSettingsOption);
    return d->defalutValue;
}

/*!
 * \brief Get current value of option.
 * \return
 */
QVariant DSettingsOption::value() const
{
    Q_D(const DSettingsOption);
    return (!d->value.isValid() || d->value.isNull()) ? d->defalutValue : d->value;
}

/*!
 * \brief Custom data of option, like QObject::property.
 * \param dataType
 * \return
 * \sa QObject::property
 * \sa Dtk::Core::DSettingsOption::setData
 */
QVariant DSettingsOption::data(const QString &dataType) const
{
    Q_D(const DSettingsOption);
    return d->datas.value(dataType);
}

/*!
 * \brief UI widget type of this option
 * \return
 * \sa Dtk::Widget::DSettingsWidgetFactory
 */
QString DSettingsOption::viewType() const
{
    Q_D(const DSettingsOption);
    return d->viewType;
}

/*!
 * \brief Check this option will show on DSettings dialog.
 * \return true if option not bind to ui element.
 */
bool DSettingsOption::isHidden() const
{
    Q_D(const DSettingsOption);
    return d->hidden;
}

/*!
 * \brief Convert QJsonObject to DSettingsOption.
 * \param prefixKey instead parse prefix key from parent.
 * \param json is an QJsonObejct instance.
 * \return
 */
QPointer<DSettingsOption> DSettingsOption::fromJson(const QString &prefixKey, const QJsonObject &json)
{
    auto optionPtr = QPointer<DSettingsOption>(new DSettingsOption);
    optionPtr->parseJson(prefixKey, json);
    return optionPtr;
}

/*!
 * \brief Set current value of option
 * \param value
 */
void DSettingsOption::setValue(QVariant value)
{
    Q_D(DSettingsOption);

    if (d->value == value) {
        return;
    }

    d->value = value;
    Q_EMIT valueChanged(value);
}

///*!
// * \brief Override default value of json
// * \param value
// */
//void DSettingsOption::setDefault(QVariant value)
//{
//    Q_D(DSettingsOption);
//    d->defalutValue = value;
//}

/*!
 * \brief Set custom data
 * \param dataType is data id, just a unique string.
 * \param value of the data id.
 * \sa Dtk::Core::DSettingsOption::data
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
 * \brief Parse QJsonObject to DSettingsOption.
 * \param prefixKey instead parse prefix key from parent.
 * \param json is an QJsonObejct instance.
 * \sa QPointer<DSettingsOption> Dtk::Core::DSettingsOption::fromJson(const QString &prefixKey, const QJsonObject &json)
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


