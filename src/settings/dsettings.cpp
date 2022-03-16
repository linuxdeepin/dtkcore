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


/*!
   \class Dtk::Core::DSettingsBackend
   \inmodule dtkcore
   \brief DSettingsBackend is interface of DSettings storage class.

   Simaple example:

\code
{
    "groups": [{
        "key": "base",
        "name": "Basic settings",
        "groups": [{
                "key": "open_action",
                "name": "Open Action",
                "options": [{
                        "key": "alway_open_on_new",
                        "type": "checkbox",
                        "text": "Always Open On New Windows",
                        "default": true
                    },
                    {
                        "key": "open_file_action",
                        "name": "Open File:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            },
            {
                "key": "new_tab_windows",
                "name": "New Tab & Window",
                "options": [{
                        "key": "new_window_path",
                        "name": "New Window Open:",
                        "type": "combobox",
                        "default": ""
                    },
                    {
                        "key": "new_tab_path",
                        "name": "New Tab Open:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            }
        ]
    }]
}
\endcode

    How to read/write key and value:

    \code
    // init a storage backend
    QTemporaryFile tmpFile;
    tmpFile.open();
    auto backend = new Dtk::Core::QSettingBackend(tmpFile.fileName());

    // read settings from json
    auto settings = Dtk::Core::DSettings::fromJsonFile(":/resources/data/dfm-settings.json");
    settings->setBackend(backend);

    // read value
    auto opt = settings->option("base.new_tab_windows.new_window_path");
    qDebug() << opt->value();

    // modify value
    opt->setValue("Test")
    qDebug() << opt->value();
   \endcode
   \sa Dtk::Core::DSettingsOption
   \sa Dtk::Core::DSettingsGroup
   \sa Dtk::Core::DSettingsBackend
   \sa Dtk::Widget::DSettingsWidgetFactory
   \sa Dtk::Widget::DSettingsDialog
 */

/*!
  \fn virtual QStringList DSettingsBackend::keys() const = 0;
  \brief return all key of storage.
  \brief 返回全部键值。
 */
/*!
  \fn virtual QVariant DSettingsBackend::getOption(const QString &key) const = 0;
  \brief get value by \a key.
  \brief 获取 \a key 对应的值。
 */
/*!
  \fn virtual void DSettingsBackend::doSync() = 0;
  \brief do the real sync action.
  \brief 开始进行同步。
 */
/*!
  \fn virtual void DSettingsBackend::doSetOption(const QString &key, const QVariant &value) = 0;
  \brief write \a key / \a value to storage.
  \brief 设置key对应的值，并使用存储后端进行存储。
 */
/*!
  \fn void DSettingsBackend::optionChanged(const QString &key, const QVariant &value);
  \brief emitted when option \a value changed.
  \brief DSettingsOption的值发生变化时发出的信号.

  \a key 发生改变的 option 键，\a value 对应键的值.
 */
/*!
  \fn void DSettingsBackend::sync();
  \brief private signal, please do not use it.
  \brief 私有信号，请勿使用。
 */
/*!
  \fn void DSettingsBackend::setOption(const QString &key, const QVariant &value);
  \brief private signal, please do not use it.
  \brief 私有信号，请勿使用.

  \internal
  \a key \a value
 */

/*!
 \class Dtk::Core::DSettings
 \inmodule dtkcore
 \brief DSettings是设计上为Dtk的应用程序提供统一的配置存储以及界面生成工具的基础库.

 DSetting使用json作为应用配置程序的描述文件。简单来说，应用查询的配置分为组/键值二个基础层级，
 对于一个标准的Dtk配置控件，一般只包含组/子组/键值三个层级，对于超过三个层级的键值，可以通过
 DSettings的API接口进行读取和写入，但是不能在标准的DSettingsDialogs上显示出来。

 一个简单的配置文件如下：
\code
{
    "groups": [{
        "key": "base",
        "name": "Basic settings",
        "groups": [{
                "key": "open_action",
                "name": "Open Action",
                "options": [{
                        "key": "alway_open_on_new",
                        "type": "checkbox",
                        "text": "Always Open On New Windows",
                        "default": true
                    },
                    {
                        "key": "open_file_action",
                        "name": "Open File:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            },
            {
                "key": "new_tab_windows",
                "name": "New Tab & Window",
                "options": [{
                        "key": "new_window_path",
                        "name": "New Window Open:",
                        "type": "combobox",
                        "default": ""
                    },
                    {
                        "key": "new_tab_path",
                        "name": "New Tab Open:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            }
        ]
    }]
}
\endcode

改组中包含一个base的root组，两个子组: open_action/new_tab_windows,每个子组有包含若干选项。
对于"New Window Open:"这个配置，其完整的访问id为base.new_tab_windows.new_window_path。
读取/设置其值的示例如下:

\code
    // 初始化一个存储后端
    QTemporaryFile tmpFile;
    tmpFile.open();
    auto backend = new Dtk::Core::QSettingBackend(tmpFile.fileName());

    // 从json中初始化配置
    auto settings = Dtk::Core::DSettings::fromJsonFile(":/resources/data/dfm-settings.json");
    settings->setBackend(backend);

    // 读取配置
    auto opt = settings->option("base.new_tab_windows.new_window_path");
    qDebug() << opt->value();

    // 修改配置
    opt->setValue("Test")
    qDebug() << opt->value();
\endcode
\sa Dtk::Core::DSettingsOption
\sa Dtk::Core::DSettingsGroup
\sa Dtk::Core::DSettingsBackend
\sa Dtk::Widget::DSettingsWidgetFactory
\sa Dtk::Widget::DSettingsDialog
*/

DSettings::DSettings(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsPrivate(this))
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
    // exit and delete thread
    connect(this, &DSettings::destroyed, this, [backendWriteThread](){
        if (backendWriteThread->isRunning()) {
            backendWriteThread->quit();
            backendWriteThread->wait();
        }
        backendWriteThread->deleteLater();
    });

    backendWriteThread->start();

    // load form backend
    loadValue();
}

/*!
   \brief 从 \a json 中获取 DSettings， 返回的数据使用之后需要自己手动释放。
 */
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
/*!
  \brief DSettings::group will recurrence find childGroup
  \a key
  \return
 */
QPointer<DSettingsGroup> DSettings::group(const QString &key) const
{
    Q_D(const DSettings);
    auto childKeylist = key.split(".");
    if (0 >= childKeylist.length()) {
        return nullptr;
    }

    auto mainGroupKey = childKeylist.value(0);
    if (1 >= childKeylist.length()) {
        return d->childGroups.value(mainGroupKey);
    }

    return d->childGroups.value(mainGroupKey)->childGroup(key);
}

QList<QPointer<DSettingsOption> > DSettings::options() const
{
    Q_D(const DSettings);
    return d->options.values();
}

QVariant DSettings::getOption(const QString &key) const
{
    QPointer<DSettingsOption> optionPointer = option(key);
    if (optionPointer) {
        return optionPointer->value();
    }
    return QVariant();
}

void DSettings::setOption(const QString &key, const QVariant &value)
{
    option(key)->setValue(value);
}

void DSettings::sync()
{
    Q_D(DSettings);
    if (!d->backend) {
        qWarning() << "backend was not setted..!";
        return;
    }

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

    if (!d->backend) {
        qWarning() << "backend was not setted..!";
        return;
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
        group->setParent(this);
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
            if (d->backend) {
                Q_EMIT d->backend->setOption(option->key(), value);
            } else {
                qWarning() << "backend was not setted..!";
            }
            Q_EMIT valueChanged(option->key(), value);
        });
    }
}

void DSettings::loadValue()
{
    Q_D(DSettings);
    if (!d->backend) {
        qWarning() << "backend was not setted..!";
        return;
    }

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
