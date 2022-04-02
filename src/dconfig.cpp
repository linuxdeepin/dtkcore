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
#else
#include <QSettings>
#endif
#include "dobject_p.h"

#include <QLoggingCategory>
#include <QCoreApplication>
#include <unistd.h>

// https://gitlabwh.uniontech.com/wuhan/se/deepin-specifications/-/issues/3

DCORE_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(cfLog)

inline static QString getAppId() {
    // TODO: 应该使用更可靠的接口获取 appId
    return QCoreApplication::applicationName();
}

/*!
    \class Dtk::Core::DConfigBackend
    \inmodule dtkcore

    \brief 配置后端的抽象接口.

    所有DConfig使用的配置后端都继承此类,用户可以继承此类实现自己的配置后端.
 */

/*!
    \fn bool DConfigBackend::load(const QString &) = 0

    \brief 初始化后端

    \a appId 管理的配置信息key值，默认为应用程序名称
  */

/*!
    \fn bool DConfigBackend::isValid() const = 0

    \sa DConfig::isValid().

 */

/*!
    \fn QStringList DConfigBackend::keyList() const = 0

    \sa DConfig::keyList()

 */

/*!
    \fn QVariant DConfigBackend::value(const QString &key, const QVariant &fallback = QVariant()) const = 0

    \sa DConfig::value()
 */

/*!
    \fn void DConfigBackend::setValue(const QString &key, const QVariant &value) = 0

    \sa DConfig::setValue()
 */

/*!
    \fn void DConfigBackend::reset(const QString &key)

    \sa DConfig::reset()
 */

/*!
    \fn QString DConfigBackend::name() const = 0

    \brief 后端配置的唯一标识

 */

DConfigBackend::~DConfigBackend()
{
}

class Q_DECL_HIDDEN DConfigPrivate : public DObjectPrivate
{
public:
    explicit DConfigPrivate(DConfig *qq,
                            const QString &appId,
                            const QString &name,
                            const QString &subpath)
        : DObjectPrivate(qq)
        , appId(appId.isEmpty() ? getAppId() : appId)
        , name(name)
        , subpath(subpath)
    {
    }

    virtual ~DConfigPrivate() override;

    inline bool invalid() const
    {
        const bool valid = backend && backend->isValid();
        if (!valid)
            qCWarning(cfLog, "DConfig is invalid of appid=%s name=%s, subpath=%s",
                      qPrintable(appId), qPrintable(name), qPrintable(subpath));

        return !valid;
    }

    DConfigBackend *getOrCreateBackend();
    DConfigBackend *createBackendByEnv();

    QString appId;
    QString name;
    QString subpath;
    QScopedPointer<DConfigBackend> backend;

    D_DECLARE_PUBLIC(DConfig)
};

namespace {

#ifndef D_DISABLE_DCONFIG
class Q_DECL_HIDDEN FileBackend : public DConfigBackend
{
public:
    explicit FileBackend(DConfigPrivate *o)
        : owner(o)
    {
    }

    virtual ~FileBackend() override;

    virtual bool isValid() const override
    {
        return configFile && configFile->isValid();
    }

    virtual bool load(const QString &appId) override
    {
        if (configFile)
            return true;

        configFile.reset(new DConfigFile(appId,owner->name, owner->subpath));
        configCache.reset(configFile->createUserCache(getuid()));
        const QString &prefix = localPrefix();

        return configFile->load(prefix) &&
               configCache->load(prefix);
    }

    virtual QStringList keyList() const override
    {
        return configFile->meta()->keyList();
    }

    virtual QVariant value(const QString &key, const QVariant &fallback) const override
    {
        const QVariant &v = configFile->value(key, configCache.get());
        return v.isValid() ? v : fallback;
    }

    virtual void setValue(const QString &key, const QVariant &value) override
    {
        if (configFile->setValue(key, value, getAppId(), configCache.get())) {
            Q_EMIT owner->q_func()->valueChanged(key);
        }
    }

    virtual void reset(const QString &key) override
    {
        const auto &originValue = configFile->meta()->value(key);
        setValue(key, originValue);
    }

    virtual QString name() const override
    {
        return QString("FileBackend");
    }

private:
    QString localPrefix() const
    {
        if (!envLocalPrefix.isEmpty()) {
            return QString::fromLocal8Bit(envLocalPrefix);
        }
        return QString();
    }

private:
    QScopedPointer<DConfigFile> configFile;
    QScopedPointer<DConfigCache> configCache;
    DConfigPrivate* owner;
    const QByteArray envLocalPrefix = qgetenv("DSG_DCONFIG_FILE_BACKEND_LOCAL_PREFIX");
};

FileBackend::~FileBackend()
{
    const QString &prefix = localPrefix();
    if (configCache) {
        configCache->save(prefix);
        configCache.reset();
    }
    if (configFile) {
        configFile->save(prefix);
        configFile.reset();
    }
}

#ifndef D_DISABLE_DBUS_CONFIG

#define DSG_CONFIG "org.desktopspec.ConfigManager"
#define DSG_CONFIG_MANAGER "org.desktopspec.ConfigManager"

class Q_DECL_HIDDEN DBusBackend : public DConfigBackend
{
public:
    explicit DBusBackend(DConfigPrivate* o):
        owner(o)
    {
    }

    virtual ~DBusBackend() override;

    static bool isServiceRegistered()
    {
        return QDBusConnection::systemBus().interface()->isServiceRegistered(DSG_CONFIG);
    }

    static bool isServiceActivatable()
    {
         const QDBusReply<QStringList> activatableNames = QDBusConnection::systemBus().interface()->
                 callWithArgumentList(QDBus::AutoDetect,
                 QLatin1String("ListActivatableNames"),
                 QList<QVariant>());
//         qInfo() << activatableNames.value() << activatableNames.value().contains(DSG_CONFIG);

         return activatableNames.value().contains(DSG_CONFIG);
    }

    virtual bool isValid() const override
    {
        return config && config->isValid();
    }

    /*!
      \internal

      初始化DBus连接,会先调用acquireManager动态获取一个配置连接,
      再通过这个配置连接进行配置文件的访问.
     */
    virtual bool load(const QString &appid) override
    {
        if (config)
            return true;

        qCDebug(cfLog, "Try acquire config manager object form DBus");
        DSGConfig dsg_config(DSG_CONFIG, "/", QDBusConnection::systemBus());
        QDBusPendingReply<QDBusObjectPath> dbus_reply = dsg_config.acquireManager(appid, owner->name, owner->subpath);
        const QDBusObjectPath dbus_path = dbus_reply.value();
        if (dbus_reply.isError() || dbus_path.path().isEmpty()) {
            qCWarning(cfLog, "Can't acquire config manager. error:\"%s\"", qPrintable(dbus_reply.error().message()));
            return false;
        } else {
            qCWarning(cfLog(), "dbus path=\"%s\"", qPrintable(dbus_path.path()));
            config.reset(new DSGConfigManager(DSG_CONFIG_MANAGER, dbus_path.path(),
                                                QDBusConnection::systemBus(), owner->q_func()));
            if (!config->isValid()) {
                qCWarning(cfLog(), "Can't acquire config path=\"%s\"", qPrintable(dbus_path.path()));
                config.reset();
                return false;
            } else {
                QObject::connect(config.data(), &DSGConfigManager::valueChanged, owner->q_func(), &DConfig::valueChanged);
            }
        }
        return true;
    }

    virtual QStringList keyList() const override
    {
        return config->keyList();
    }

    static QVariant decodeQDBusArgument(const QVariant &v)
    {
        if (v.canConvert<QDBusArgument>()) {
            // we use QJsonValue to resolve all data type in DConfigInfo class, so it's type is equal QJsonValue::Type,
            // now we parse Map and Array type to QVariant explicitly.
            const QDBusArgument &complexType = v.value<QDBusArgument>();
            switch (complexType.currentType()) {
            case QDBusArgument::MapType: {
                QVariantMap list;
                complexType >> list;
                QVariantMap res;
                for (auto iter = list.begin(); iter != list.end(); iter++) {
                    res[iter.key()] = decodeQDBusArgument(iter.value());
                }
                return res;
            }
            case QDBusArgument::ArrayType: {
                QVariantList list;
                complexType >> list;
                QVariantList res;
                res.reserve(list.size());
                for (const auto &item : qAsConst(list)) {
                    res << decodeQDBusArgument(item);
                }
                return res;
            }
            default:
                qWarning("Can't parse the type, it maybe need user to do it, "
                         "QDBusArgument::ElementType: %d.", complexType.currentType());
            }
        }
        return v;
    }

    virtual QVariant value(const QString &key, const QVariant &fallback) const override
    {
        const QDBusVariant &dv = config->value(key);
        const QVariant &v = dv.variant();
        return v.isValid() ? decodeQDBusArgument(v) : fallback;
    }

    virtual void setValue(const QString &key, const QVariant &value) override
    {
        config->setValue(key, QDBusVariant(value));
    }

    virtual void reset(const QString &key) override
    {
        config->reset(key);
    }

    virtual QString name() const override
    {
        return QString("DBusBackend");
    }

private:
    QScopedPointer<DSGConfigManager> config;
    DConfigPrivate* owner;
};

DBusBackend::~DBusBackend()
{
    if (config) {
        config->release();
    }
}
#endif //D_DISABLE_DBUS_CONFIG
#else

class Q_DECL_HIDDEN QSettingBackend : public DConfigBackend
{
public:
    explicit QSettingBackend(DConfigPrivate* o):
        owner(o)
    {
    }

    virtual ~QSettingBackend() override;

    virtual bool isValid() const override
    {
        return settings;
    }

    virtual bool load(const QString &appid) override
    {
        Q_UNUSED(appid);

        if (settings)
            return true;

        settings = new QSettings(owner->name, QSettings::IniFormat, owner->q_func());
        settings->beginGroup(owner->subpath);
        return true;
    }

    virtual QStringList keyList() const override
    {
        return settings->childKeys();
    }

    virtual QVariant value(const QString &key, const QVariant &fallback) const override
    {
        return settings->value(key, fallback);
    }

    virtual void setValue(const QString &key, const QVariant &value) override
    {
        settings->setValue(key, value);
    }

    virtual QString name() const override
    {
        return QString("QSettingBackend");
    }

private:
    QSettings *settings = nullptr;
    DConfigPrivate* owner;
};

QSettingBackend::~QSettingBackend()
{
}

#endif //D_DISABLE_DCONFIG
}

DConfigPrivate::~DConfigPrivate()
{
    backend.reset();
}

/*!
  \internal

    \brief 创建一个配置后端

    默认使用的配置后端会优先根据环境变量来选择配置中心的D-Bus接口还是文件配置后端接口。
    若没有配置此环境变量，则根据是否有配置中心提供D-Bus服务来选择配置中心服务还是文件配置后端接口.
 */
DConfigBackend *DConfigPrivate::getOrCreateBackend()
{
    if (backend) {
        return backend.data();
    }
    if (auto backendEnv = createBackendByEnv()) {
        backend.reset(backendEnv);
        return backend.data();
    }
#ifndef D_DISABLE_DCONFIG
#ifndef D_DISABLE_DBUS_CONFIG
    if (DBusBackend::isServiceRegistered() || DBusBackend::isServiceActivatable()) {
        qCDebug(cfLog, "Fallback to DBus mode");
        backend.reset(new DBusBackend(this));
    }
    if (!backend) {
        qCDebug(cfLog, "Can't use DBus config service, fallback to DConfigFile mode");
        backend.reset(new FileBackend(this));
    }
#else
    backend.reset(new FileBackend(this));
#endif //D_DISABLE_DBUS_CONFIG
#else
    qCDebug(cfLog, "Fallback to QSettings mode");
    backend.reset(new QSettingBackend(this));
#endif //D_DISABLE_DCONFIG
    return backend.data();
}

/*!
  \internal

    \brief 创建一个配置后端

    尝试根据环境变量来选择配置中心的D-Bus接口还是文件配置后端接口。
 */
DConfigBackend *DConfigPrivate::createBackendByEnv()
{
    const QByteArray &envBackend = qgetenv("DSG_DCONFIG_BACKEND_TYPE");
    if (!envBackend.isEmpty()) {
        if (envBackend == "DBusBackend") {

#ifndef D_DISABLE_DCONFIG
#ifndef D_DISABLE_DBUS_CONFIG
            if (DBusBackend::isServiceRegistered() || DBusBackend::isServiceActivatable()) {
                qCDebug(cfLog, "Fallback to DBus mode");
                return new DBusBackend(this);
            }
#endif //D_DISABLE_DBUS_CONFIG
#endif //D_DISABLE_DCONFIG
        } else if (envBackend == "FileBackend") {

#ifndef D_DISABLE_DCONFIG
            qCDebug(cfLog, "Fallback to DConfigFile mode");
            return new FileBackend(this);
#endif //D_DISABLE_DCONFIG
        } else {

#ifndef D_DISABLE_DCONFIG
#else
            qCDebug(cfLog, "Fallback to QSettings mode");
            return new QSettingBackend(this);
#endif //D_DISABLE_DCONFIG
        }
    }
    return nullptr;
}

/*!
    \class Dtk::Core::DConfig
    \inmodule dtkcore

    \brief 配置策略提供的接口类

    此接口规范定义了开发库所提供的关于配置文件读写的相关接口，
    如果应用程序所使用的开发库实现了此规范，则程序应当优先使用开发库提供的接口。
 */


/*!
 * \brief 构造配置策略提供的对象
 * \a name 配置文件名
 * \a subpath 配置文件对应的子目录
 * \a parent 父对象
 */
DConfig::DConfig(const QString &name, const QString &subpath, QObject *parent)
    : DConfig(nullptr, name, subpath, parent)
{
}

DConfig::DConfig(DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent)
    : DConfig(backend, QString(), name, subpath, parent)
{

}
/*!
 * \brief 构造配置策略提供的对象, 指定配置所属的应用Id
 * \a appId
 * \a name
 * \a subpath
 * \a parent
 * \return 构造的配置策略对象，由调用者释放
 */
DConfig *DConfig::create(const QString &appId, const QString &name, const QString &subpath, QObject *parent)
{
    return new DConfig(nullptr, appId, name, subpath, parent);
}

DConfig *DConfig::create(DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent)
{
    return new DConfig(backend, appId, name, subpath, parent);
}

/*!
 * \brief 使用自定义的配置策略后端构造对象
 * \a backend 调用者继承于DConfigBackend的配置策略后端
 * \a appId 配置文件所属的应用Id，为空时默认为本应用Id
 * \a name 配置文件名
 * \a subpath 配置文件对应的子目录
 * \a parent 父对象
 * \note 调用者只构造backend，由DConfig释放。
 */
DConfig::DConfig(DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent)
    : QObject(parent)
    , DObject(*new DConfigPrivate(this, appId, name, subpath))
{
    D_D(DConfig);

    Q_ASSERT(!d->appId.isEmpty());

    qCDebug(cfLog, "Load config of appid=%s name=%s, subpath=%s",
            qPrintable(d->appId), qPrintable(d->name), qPrintable(d->subpath));

    if (backend) {
        d->backend.reset(backend);
    }

    if (auto backend = d->getOrCreateBackend()) {
        backend->load(d->appId);
    }
}

/*!
 * \brief DConfig::backendName
 * \return 配置策略后端名称
 * \note 调用者只能用DConfig访问DConfigBackend对象，所以不返回DConfigBackend对象。
 */
QString DConfig::backendName() const
{
    D_DC(DConfig);
    if (d->invalid())
        return QString();

    return d->backend->name();
}

/*!
 * \brief 获得所有可用的配置项名称
 * \return 配置项名称集合
 */
QStringList DConfig::keyList() const
{
    D_DC(DConfig);
    if (d->invalid())
        return QStringList();

    return d->backend->keyList();
}

/*!
 * \brief 判断此后端是否可用
 * \return
 */
bool DConfig::isValid() const
{
    D_DC(DConfig);
    return !d->invalid();
}

/*!
 * \brief 根据配置项名称获得对应值
 * \param key 配置项名称
 * \param fallback 没有获取到配置项值后提供的默认值
 * \return
 */
QVariant DConfig::value(const QString &key, const QVariant &fallback) const
{
    D_DC(DConfig);
    if (d->invalid())
        return fallback;

    return d->backend->value(key, fallback);
}

/*!
 * \brief 根据配置项名称设置其值
 * \param 配置项名称
 * \param 需要更新的值
 */
void DConfig::setValue(const QString &key, const QVariant &value)
{
    D_D(DConfig);
    if (d->invalid())
        return;

    d->backend->setValue(key, value);
}

/*!
 * \brief 设置其配置项对应的默认值，此值为经过override机制覆盖后的值，不一定为此配置文件中meta中定义的值
 * \param 配置项名称
 */
void DConfig::reset(const QString &key)
{
    D_D(DConfig);
    if (d->invalid())
        return;

    d->backend->reset(key);
}

/*!
 * \brief 返回配置文件名称
 * \return
 */
QString DConfig::name() const
{
    D_DC(DConfig);
    return d->name;
}

/*!
 * \brief 返回配置文件对应的子目录
 * \return
 */
QString DConfig::subpath() const
{
    D_DC(DConfig);
    return d->subpath;
}

DCORE_END_NAMESPACE
