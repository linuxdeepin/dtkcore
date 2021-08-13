/*
 * Copyright (C) 2021 Uniontech Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@uniontech.com>
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
#include "dconfig.h"
#ifndef D_DISABLE_DCONFIG
#include "dconfigfile.h"
#ifndef D_DISABLE_DBUS_CONFIG
#include "configmanager_interface.h"
#include "manager_interface.h"
#endif
#include <unistd.h>
#include <pwd.h>
#else
#include <QSettings>
#endif
#include "dobject_p.h"

#include <QLoggingCategory>
#include <QCoreApplication>

// https://gitlabwh.uniontech.com/wuhan/se/deepin-specifications/-/issues/3

DCORE_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(cfLog)

#define DSG_CONFIG "org.desktopspec.ConfigManager"
#define DSG_CONFIG_MANAGER "org.desktopspec.ConfigManager"

inline static QString getAppId() {
    // TODO: 应该使用更可靠的接口获取 appid
    return QCoreApplication::applicationName();
}

class Q_DECL_HIDDEN DConfigPrivate : public DObjectPrivate
{
public:
    DConfigPrivate(DConfig *qq)
        : DObjectPrivate(qq)
    {

    }

    QString name;
    QString subpath;
#ifndef D_DISABLE_DCONFIG
    inline static QString getUserName() {
        uid_t uid = geteuid();
        passwd *pw = getpwuid(uid);
        return pw ? QString::fromLocal8Bit(pw->pw_name) : QString();
    }

#ifndef D_DISABLE_DBUS_CONFIG
    DSGConfigManager *config = nullptr;
#endif
    QScopedPointer<DConfigFile> configFile;

    inline QStringList keyList() const {
#ifndef D_DISABLE_DBUS_CONFIG
        if (Q_LIKELY(config)) {
            return config->keyList();
        }
#endif
        return configFile->keyList();
    }
    inline QVariant value(const QString &key, const QVariant &fallback) const {
#ifndef D_DISABLE_DBUS_CONFIG
        if (Q_LIKELY(config)) {
            const QDBusVariant &dv = config->value(key);
            const QVariant &v = dv.variant();
            return v.isValid() ? v : fallback;
        }
#endif
        return configFile->value(key, fallback);
    }
    inline void setValue(const QString &key, const QVariant &value) {
#ifndef D_DISABLE_DBUS_CONFIG
        if (Q_LIKELY(config)) {
            config->setValue(key, QDBusVariant(value));
            return;
        }
#endif
        configFile->setValue(key, value, getUserName(), getAppId());
    }
#else
    QSettings *settings = nullptr;

    inline QStringList keyList() const {
        return settings->childKeys();
    }
    inline QVariant value(const QString &key, const QVariant &fallback) const {
        return settings->value(key, fallback);
    }
    inline void setValue(const QString &key, const QVariant &value) {
        settings->setValue(key, value);
    }
#endif

    D_DECLARE_PUBLIC(DConfig)
};

DConfig::DConfig(const QString &name, const QString &subpath,
                 QObject *parent)
    : QObject(parent)
    , DObject(*new DConfigPrivate(this))
{
    D_D(DConfig);
    d->name = name;
    d->subpath = subpath;
}

bool DConfig::load()
{
    D_D(DConfig);

    const auto &appid = getAppId();
    Q_ASSERT(!appid.isEmpty());

    qCDebug(cfLog, "Load config of appid=%s name=%s, subpath=%s",
            qPrintable(appid), qPrintable(d->name), qPrintable(d->subpath));

#ifndef D_DISABLE_DCONFIG
    bool use_dbus_config = false;
#ifndef D_DISABLE_DBUS_CONFIG
    if (d->config)
        return true;

    use_dbus_config = QDBusConnection::systemBus().interface()->isServiceRegistered(DSG_CONFIG);

    if (use_dbus_config) {
        qCDebug(cfLog, "Try acquire config manager object form DBus");
        DSGConfig dsg_config(DSG_CONFIG, "/", QDBusConnection::systemBus());
        const QDBusObjectPath dbus_path = dsg_config.acquireManager(appid, d->name, d->subpath);
        if (dbus_path.path().isEmpty()) {
            qCWarning(cfLog, "Can't acquire config manager");
            return false;
        } else {
            d->config = new DSGConfigManager(DSG_CONFIG_MANAGER, dbus_path.path(),
                                             QDBusConnection::systemBus(), this);
            if (!d->config->isValid()) {
                delete d->config;
                d->config = nullptr;
                return false;
            } else {
                connect(d->config, &DSGConfigManager::valueChanged, this, &DConfig::valueChanged);
                use_dbus_config = true;
            }
        }
    }
#endif

    if (!use_dbus_config) {
        qCDebug(cfLog, "Can't use DBus config service, fallback to DConfigFile mode");
        if (d->configFile)
            return true;
        d->configFile.reset(new DConfigFile(appid, d->name, d->subpath));
    }
#else
    qCDebug(cfLog, "Fallback to QSettings mode");
    if (d->settings)
        return true;

    d->settings = new QSettings(d->name, QSettings::IniFormat, this);
    d->settings->beginGroup(d->subpath);
#endif

    return true;
}

QStringList DConfig::keyList() const
{
    D_DC(DConfig);
    return d->keyList();
}

bool DConfig::isValid() const
{
    D_DC(DConfig);
#ifndef D_DISABLE_DCONFIG
    if (d->configFile)
        return true;

#ifndef D_DISABLE_DBUS_CONFIG
    return d->config;
#endif
#else
    return d->settings;
#endif
}

QVariant DConfig::value(const QString &key, const QVariant &fallback) const
{
    D_DC(DConfig);
    return d->value(key, fallback);
}

void DConfig::setValue(const QString &key, const QVariant &value)
{
    D_D(DConfig);
    d->setValue(key, value);
}

DCORE_END_NAMESPACE
