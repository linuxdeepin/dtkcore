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

#include "dsettings.h"

#include <QMap>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>
#include <QDebug>

#include "dsettingsoption.h"
#include "dsettingsgroup.h"
#include "dsettingsbackend.h"

DCORE_BEGIN_NAMESPACE

class DSettingsPrivate
{
public:
    DSettingsPrivate(DSettings *parent) : q_ptr(parent) {}

    DSettingsBackend            *backend = nullptr;
    QJsonObject                 meta;
    QMap <QString, OptionPtr>   options;

    QMap<QString, GroupPtr>     childGroups;
    QList<QString>              childGroupKeys;

    DSettings *q_ptr;
    Q_DECLARE_PUBLIC(DSettings)
};


DSettings::DSettings(QObject *parent) :
    QObject(parent), d_ptr(new DSettingsPrivate(this))
{
}

DSettings::~DSettings()
{

}

void DSettings::setBackend(DSettingsBackend *backend)
{
    Q_D(DSettings);
    if (nullptr == backend) {
        return;
    }

    if (d->backend != nullptr) {
        qWarning() << "set backend to exist " << d->backend;
    }

    d->backend = backend;


    auto backendWriteThread = new QThread;
    d->backend->moveToThread(backendWriteThread);

    connect(d->backend, &DSettingsBackend::optionChanged,
    this, [ = ](const QString & key, const QVariant & value) {
        option(key)->setValue(value);
    });

    backendWriteThread->start();

    // load form backend
    loadValue();
}

QPointer<DSettings> DSettings::fromJson(const QByteArray &json)
{
    auto settingsPtr = QPointer<DSettings>(new DSettings);
    settingsPtr->parseJson(json);
    return settingsPtr;
}

QPointer<DSettings> DSettings::fromJsonFile(const QString &filepath)
{
    QFile jsonFile(filepath);
    jsonFile.open(QIODevice::ReadOnly);
    auto jsonData = jsonFile.readAll();
    jsonFile.close();

    return DSettings::fromJson(jsonData);
}

QJsonObject DSettings::meta() const
{
    Q_D(const DSettings);
    return d->meta;
}

QStringList DSettings::keys() const
{
    Q_D(const DSettings);
    return d->options.keys();
}

QPointer<DSettingsOption> DSettings::option(const QString &key) const
{
    Q_D(const DSettings);
    return d->options.value(key);
}

QVariant DSettings::value(const QString &key) const
{
    Q_D(const DSettings);
    auto opt = d->options.value(key);
    if (opt.isNull()) {
        return QVariant();
    }

    return opt->value();
}

QStringList DSettings::groupKeys() const
{
    Q_D(const DSettings);
    return d->childGroupKeys;
}

QList<QPointer<DSettingsGroup> > DSettings::groups() const
{
    Q_D(const DSettings);
    return d->childGroups.values();
}

QPointer<DSettingsGroup> DSettings::group(const QString &key) const
{
    Q_D(const DSettings);
    return d->childGroups.value(key);
}

QList<QPointer<DSettingsOption> > DSettings::options() const
{
    Q_D(const DSettings);
    return d->options.values();
}

QVariant DSettings::getOption(const QString &key) const
{
    return option(key)->value();
}

void DSettings::setOption(const QString &key, const QVariant &value)
{
    option(key)->setValue(value);
}

void DSettings::sync()
{
    Q_D(DSettings);
    d->backend->doSync();
}

void DSettings::reset()
{
    Q_D(DSettings);

    for (auto option : d->options) {
        if (option->canReset()) {
            setOption(option->key(), option->defaultValue());
        }
    }
    d->backend->sync();
}

void DSettings::parseJson(const QByteArray &json)
{
    Q_D(DSettings);

    auto jsonDoc = QJsonDocument::fromJson(json);
    d->meta = jsonDoc.object();
    auto mainGroups = d->meta.value("groups");
    for (auto groupJson : mainGroups.toArray()) {
        auto group = DSettingsGroup::fromJson("", groupJson.toObject());
        for (auto option : group->options()) {
            d->options.insert(option->key(), option);
        }
        d->childGroupKeys << group->key();
        d->childGroups.insert(group->key(), group);
    }

    for (auto option :  d->options.values()) {
        d->options.insert(option->key(), option);
        connect(option.data(), &DSettingsOption::valueChanged,
        this, [ = ](QVariant value) {
            Q_EMIT d->backend->setOption(option->key(), value);
            Q_EMIT valueChanged(option->key(), value);
        });
    }
}

void DSettings::loadValue()
{
    Q_D(DSettings);

    for (auto key : d->backend->keys()) {
        auto value = d->backend->getOption(key);
        auto opt = option(key);
        if (!value.isValid() || opt.isNull()) {
            continue;
        }

        opt->blockSignals(true);
        opt->setValue(value);
        opt->blockSignals(false);
    }
}

DCORE_END_NAMESPACE
