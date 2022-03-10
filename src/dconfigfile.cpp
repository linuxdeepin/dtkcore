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
#include "dconfigfile.h"

#include "dobject_p.h"
#include "filesystem/dstandardpaths.h"

#include <QFile>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QVariant>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QCollator>
#include <QDateTime>

#include <unistd.h>
#include <pwd.h>

// https://gitlabwh.uniontech.com/wuhan/se/deepin-specifications/-/issues/3

DCORE_BEGIN_NAMESPACE

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(cfLog, "dtk.dsg.config" , QtInfoMsg);
#else
Q_LOGGING_CATEGORY(cfLog, "dtk.dsg.config");
#endif

#define FILE_SUFFIX QLatin1String(".json")

/*!
  \internal

    \brief 按子目录查找机制查找配置文件

    在 \a baseDir目录下,查找名称为 \a name的文件,
    若存在 \a subpath,则从 \a subpath叶子目录逐级向上查找名称为 \a name的文件,
    若不存在此文件,则返回无效路径.
 */
inline QString getFile(const QString &baseDir, const QString &subpath, const QString &name,
                       bool canFallbackUp = true) {
    qCDebug(cfLog, "load json file from base dir:\"%s\", subpath = \"%s\", file name =\"%s\".",
            qPrintable(baseDir), qPrintable(subpath), qPrintable(name));

    const QDir base_dir(baseDir);
    QDir target_dir = base_dir;

    if (!subpath.isEmpty())
        target_dir.cd(subpath.mid(1));

    do {
        qCDebug(cfLog, "load json file from: \"%s\"", qPrintable(target_dir.path()));

        if (QFile::exists(target_dir.filePath(name))) {
            return target_dir.filePath(name);
        }

        if (base_dir == target_dir)
            break;
    } while (canFallbackUp && target_dir.cdUp());

    return QString();
}

inline QFile *loadFile(const QString &baseDir, const QString &subpath, const QString &name,
                       bool canFallbackUp = true)
{
    QString path = getFile(baseDir, subpath, name, canFallbackUp);
    if (!path.isEmpty()) {
        return new QFile(path);
    }
    return nullptr;
}

static QJsonDocument loadJsonFile(QIODevice *data)
{
    if (!data->open(QIODevice::ReadOnly)) {
        if (auto file = qobject_cast<QFile*>(data)) {
            qCDebug(cfLog, "Falied on open file: \"%s\", error message: \"%s\"",
                    qPrintable(file->fileName()), qPrintable(file->errorString()));
        }
        return QJsonDocument();
    }

    QJsonParseError error;
    auto document = QJsonDocument::fromJson(data->readAll(), &error);
    data->close();

    if (error.error != QJsonParseError::NoError) {
        qCWarning(cfLog, "%s", qPrintable(error.errorString()));
        return QJsonDocument();
    }

    return document;
}

static DConfigFile::Version parseVersion(const QJsonObject &obj) {
    DConfigFile::Version version {0, 0};
    const QString &verStr = obj[QLatin1String("version")].toString();

    if (verStr.isEmpty()) {
        return version;
    }

    const QStringList &items = verStr.split(QLatin1Char('.'));

    if (items.size() != 2)
        return version;

    bool ok = false;
    quint16 major = items.first().toUShort(&ok);

    if (!ok)
        return version;

    quint16 minor = items.last().toUShort(&ok);

    if (!ok)
        return version;

    version.major = major;
    version.minor = minor;

    return version;
}

#define MAGIC_META QLatin1String("dsg.config.meta")
#define MAGIC_OVERRIDE QLatin1String("dsg.config.override")
#define MAGIC_CACHE QLatin1String("dsg.config.cache")

static const uint GlobalUID = 0xFFFF;

inline static bool checkMagic(const QJsonObject &obj, QLatin1String request) {
    return obj[QLatin1String("magic")].toString() == request;
}

inline static bool versionIsValid(const DConfigFile::Version &v) {
    return v.major > 0 || v.minor > 0;
}

inline static bool checkVersion(const QJsonObject &obj, const DConfigFile::Version &request) {
    const DConfigFile::Version &v = parseVersion(obj);
    return versionIsValid(v) && v.major == request.major;
}

inline void overrideValue(QLatin1String subkey, const QJsonValue &from, QVariantHash &target) {
    const QJsonValue &v = from[subkey];

    if (!v.isUndefined())
        target[subkey] = v.toVariant();
}

inline static QString getUserName(const uint uid) {
    passwd *pw = getpwuid(uid);
    return pw ? QString::fromLocal8Bit(pw->pw_name) : QString();
}

/*!
    \class Dtk::Core::DConfigFile
    \inmodule dtkcore

    \brief 规范配置文件读写的相关接口的配置文件实现.
 */

/*!
    \enum DConfigFile::Flag

    \value NoOverride  存在此标记时，将表明则此配置项不可被覆盖（详见下述 override 机制）。
    反之，不存在此标记时表明此配置项允许被覆盖，对于此类配置项，
    如若其有界面设置入口，则当此项不可写时，应当隐藏或禁用界面的设置入口.
    \value Global 当读写此类配置时，将忽略用户身份，无论程序使用哪个用户身份执行，读操作都将获取到同样的数据，
    写操作将对所有用户都生效。但是，如果对应的配置存储目录不存在或无权限写入，则忽略此标志.
*/

/*!
    \enum DConfigFile::Permissions

    \value ReadOnly 将配置项覆盖为只读，
    \value ReadWrite 将配置项覆盖为可读可写.
*/

/*!
    \enum DConfigFile::Visibility

    \value Private 仅限程序内部使用，
    对外不可见。此类配置项完全由程序自己读写，可随意增删改写其含义，无需做兼容性考虑，
    \value Public 外部程序可使用。
    此类配置项一旦发布，在兼容性版本的升级中，要保障此配置项向下兼容，
    简而言之，只允许在程序/库的大版本升级时才允许删除或修改此类配置项，
    当配置项的 permissions、visibility、flags 任意一个属性被修改则认为此配置项被修改，
    除此之外修改 value、name、description 属性时则不需要考虑兼容性.
*/

/*!
    \struct Dtk::Core::DConfigFile::Version
    \inmodule dtkcore
    \brief 版本信息

    此文件的内容格式的版本。版本号使用两位数字描述，
    首位数字不同的描述文件相互之间不兼容，第二位数字不同的描述文件需满足向下兼容。
    读取此描述文件的程序要根据版本进行内容分析，当遇到不兼容的版本时，需要立即终止解析，忽略此文件，
    并在程序日志中写入警告信息，如 “1.0” 和 “2.0” 版本之间不兼容，
    如果解析程序最高只支持 1.0 版本，则遇到 2.0 版本的描述文件时应该终止解析，
    但是如果遇到 1.1 版本，则可以继续执行。
    写入此描述文件时，遇到不兼容的版本时，需要先清空当前内容再写入，每次写入皆需更新此字段。
*/

DConfigMeta::~DConfigMeta() {}

Dtk::Core::DConfigCache::~DConfigCache() {}

struct DConfigKey {
    DConfigKey(const QString &aappId, const QString &afileName, const QString &asubpath)
        : appId(aappId),
          fileName(afileName),
          subpath(asubpath)
    {
    }

    explicit DConfigKey(const DConfigKey &src)
        : DConfigKey(src.appId, src.fileName, src.subpath)
    {
    }

    DConfigKey &operator = (const DConfigKey &src)
    {
        this->appId = src.appId;
        this->fileName = src.fileName;
        this->subpath = src.subpath;
        return *this;
    }

    QString appId;
    QString fileName;
    QString subpath;
};

class Q_DECL_HIDDEN DConfigInfo {
public:
    DConfigInfo()
    {

    }
    DConfigInfo(const DConfigInfo &other)
    {
        this->values = other.values;
    }
    DConfigInfo operator = (const DConfigInfo &other)
    {
        this->values = other.values;
        return *this;
    }
    inline static bool checkSerial(const int metaSerial, const int cacheSerial)
    {
        if (cacheSerial < 0)
            return true;
        if (metaSerial >= 0 && metaSerial == cacheSerial)
            return true;
        return false;
    }

    DConfigFile::Visibility visibility(const QString &key) const
    {
        DConfigFile::Visibility p = DConfigFile::Private;
        const auto &tmp = values[key][QLatin1String("visibility")].toString();
        if (tmp == QLatin1String("public"))
            p = DConfigFile::Public;

        return p;
    }

    DConfigFile::Permissions permissions(const QString &key) const
    {
        DConfigFile::Permissions p = DConfigFile::ReadOnly;
        const auto &tmp = values[key][QLatin1String("permissions")].toString();
        if (tmp == QLatin1String("readwrite"))
            p = DConfigFile::ReadWrite;

        return p;
    }

    DConfigFile::Flags flags(const QString &key) const
    {
        DConfigFile::Flags flags = {};
        const auto &tmp = values[key][QLatin1String("flags")];
        Q_FOREACH(const QString &flag, tmp.toStringList()) {
            if (flag == QLatin1String("nooverride")) {
                flags |= DConfigFile::NoOverride;
            } else if (flag == QLatin1String("global")) {
                flags |= DConfigFile::Global;
            }
        }

        return flags;
    }

    QString displayName(const QString &key, const QLocale &locale) const
    {
        if (locale == QLocale::AnyLanguage)
            return values[key][QLatin1String("name")].toString();

        return values[key].value(QString("name[%1]")
                                 .arg(locale.name())).toString();
    }

    QString description(const QString &key, const QLocale &locale) const
    {
        if (locale == QLocale::AnyLanguage)
            return values[key][QLatin1String("description")].toString();

        return values[key].value(QString("description[%1]")
                                 .arg(locale.name())).toString();
    }

    inline QVariant value(const QString &key) const
    {
        return values[key][QLatin1String("value")];
    }

    inline int serial(const QString &key) const
    {
        bool status = false;
        const int tmp = values[key][QLatin1String("serial")].toInt(&status);
        if (status) {
            return tmp;
        }
        return -1;
    }

    inline void setValue(const QString &key, const QVariant &value)
    {
        values[key]["value"] = value;
    }

    inline void setSerial(const QString &key, const int &value)
    {
        values[key]["serial"] = value;
    }

    inline void setTime(const QString &key, const QString &value)
    {
        values[key]["time"] = value;
    }

    inline void setUser(const QString &key, const uint &value)
    {
        values[key]["user"] = getUserName(value);
    }

    inline void setAppId(const QString &key, const QString &value)
    {
        values[key]["appid"] = value;
    }

    inline QStringList keyList() const
    {
        return values.keys();
    }

    inline void remove(const QString &key)
    {
        values.remove(key);
    }

    inline void update(const QString &key, const QVariantHash &value)
    {
        values[key] = value;
    }

    inline void updateValue(const QString &key, const QJsonValue &value)
    {
        overrideValue(key, "value", value);
    }

    inline void updateSerial(const QString &key, const QJsonValue &value)
    {
        overrideValue(key, "serial", value);
    }

    inline void updatePermissions(const QString &key, const QJsonValue &value)
    {
        overrideValue(key, "permissions", value);
    }

    QJsonObject content() const
    {
        QJsonObject contents;
        for (auto i = values.constBegin(); i != values.constEnd(); ++i) {
            contents[i.key()] = QJsonObject::fromVariantHash(i.value());
        }
        return contents;
    }
private:
    void overrideValue(const QString &key, const QString &subkey, const QJsonValue &from) {
        const QJsonValue &v = from[subkey];

        if (!v.isUndefined())
            values[key][subkey] = v.toVariant();
    }

    QHash<QString, QVariantHash> values;
};


/*!
    \class Dtk::Core::DConfigMeta
    \inmodule dtkcore

    \brief 提供配置文件的原型和覆盖机制的访问接口.

*/

/*!
    \fn DConfigFile::Version DConfigMeta::version() const = 0;

    \brief 返回配置版本信息.
    \return
*/

/*!
    \fn void DConfigMeta::setVersion(quint16 major, quint16 minor) = 0;

    \brief 设置配置版本信息
    \a major 主板本号
    \a minor 次版本号
*/

/*!
    \fn bool DConfigMeta::load(const QString &localPrefix = QString()) = 0;

    \brief 解析配置文件
    \a localPrefix 为目录前缀
    \return
*/

/*!
    \fn bool DConfigMeta::load(QIODevice *meta, const QList<QIODevice*> &overrides) = 0;

    \brief 解析配置文件流
    \a meta 为原型流
    \a overrides 为覆盖机制查找的文件流
    \return
*/

/*!
    \fn QStringList DConfigMeta::keyList() const = 0;

    \brief 返回配置内容的所有配置项
    \return
*/

/*!
    \fn DConfigFile::Flags DConfigMeta::flags(const QString &key) const = 0;

    \brief 返回指定配置项的特性
    \a key 配置项名称, NoOverride为此配置项不可被覆盖, Global为忽略用户身份
    \return
*/

/*!
    \fn DConfigFile::Permissions DConfigMeta::permissions(const QString &key) const = 0;

    \brief 返回指定配置项的权限
    \a key 配置项名称
    \return

*/

/*!
    \fn DConfigFile::Visibility DConfigMeta::visibility(const QString &key) const = 0;

    \brief 返回指定配置项的可见性
    \a key 配置项名称
    \return

*/

/*!
    \fn int DConfigMeta::serial(const QString &key) const = 0;

    \brief 返回配置项的单调递增值
    \a key 配置项名称
    \return -1为无效值，表明没有配置此项
*/

/*!
    \fn QString DConfigMeta::displayName(const QString &key, const QLocale &locale) = 0;

    \brief 返回指定配置项的显示名
    \a key 配置项名称
    \a locale 为语言版本
    \return
*/

/*!
    \fn QString DConfigMeta::description(const QString &key, const QLocale &locale) = 0;

    \brief 返回指定配置项的描述信息
    \a key 配置项名称
    \a locale 为语言版本
    \return

*/

/*!
    \fn QString DConfigMeta::metaPath(const QString &localPrefix = QString(), bool *useAppId = nullptr) const = 0;

    \brief 返回描述文件的路径
    \a localPrefix 目录的所有需要查找的覆盖机制目录
    \return
*/

/*!
    \fn QStringList DConfigMeta::allOverrideDirs(const bool useAppId, const QString &prefix = QString()) const = 0;

    \brief 获得前缀为 \a prefix 目录的所有需要查找的覆盖机制目录
    \a userAppId 是否不使用通用目录
    \return
*/

/*!
    \fn QVariant DConfigMeta::value(const QString &key) const = 0;

    \brief meta初始值经过覆盖机制覆盖后的原始值
    \a key 配置项名称
    \return
*/

class Q_DECL_HIDDEN DConfigMetaImpl : public DConfigMeta {
    // DConfigMeta interface
public:
    explicit DConfigMetaImpl(const DConfigKey &configKey);
    virtual ~DConfigMetaImpl() override;

    inline virtual QStringList keyList() const override
    {
        return values.keyList();
    }
    inline virtual DConfigFile::Flags flags(const QString &key) const override
    {
        return values.flags(key);
    }
    inline virtual DConfigFile::Permissions permissions(const QString &key) const override
    {
        return values.permissions(key);
    }
    inline virtual DConfigFile::Visibility visibility(const QString &key) const override
    {
        return values.visibility(key);
    }
    inline virtual int serial(const QString &key) const override
    {
        return values.serial(key);
    }
    inline virtual QString description(const QString &key, const QLocale &locale) override
    {
        return values.description(key, locale);
    }
    virtual DConfigFile::Version version() const override
    {
        return m_version;
    }
    inline virtual void setVersion(quint16 major, quint16 minor) override
    {
        m_version.major = major;
        m_version.minor = minor;
    }
    inline virtual QString displayName(const QString &key, const QLocale &locale) override
    {
        return values.displayName(key, locale);
    }
    inline virtual QVariant value(const QString &key) const override
    {
        return values.value(key);
    }

    inline QStringList applicationMetaDirs(const QString &prefix) const
    {
        QStringList paths;
        // lower priority is higher.
        const auto &dataPaths = DStandardPaths::paths(DStandardPaths::DSG::DataDir);
        paths.reserve(dataPaths.size());
        for (auto item : dataPaths) {
            paths.prepend(QString("%1/%2/configs/%3").arg(prefix, item, configKey.appId));
        }
        return paths;
    }

    inline static QStringList genericMetaDirs(const QString &prefix) {
        QStringList paths;
        for (auto item: DStandardPaths::paths(DStandardPaths::DSG::DataDir)) {
            paths.prepend(QString("%1/%2/configs").arg(prefix, item));
        }
        return paths;
    }

    QString metaPath(const QString &localPrefix, bool *useAppId) const override
    {
        bool useAppIdForOverride = true;

        QString path;
        const QStringList &applicationMetas = applicationMetaDirs(localPrefix);
        for (auto iter = applicationMetas.rbegin(); iter != applicationMetas.rend(); iter++) {
            path = getFile(*iter, configKey.subpath, configKey.fileName + FILE_SUFFIX);
            if (!path.isEmpty())
                break;
        }

        if (path.isEmpty()) {
            useAppIdForOverride = false;
            const QStringList &genericnMetas = genericMetaDirs(localPrefix);
            for (auto iter = genericnMetas.rbegin(); iter != genericnMetas.rend(); iter++) {
                path = getFile(*iter, configKey.subpath, configKey.fileName + FILE_SUFFIX);
                if (!path.isEmpty())
                    break;
            }
        }
        if (useAppId) {
            *useAppId = useAppIdForOverride;
        }
        return path;
    }

    bool load(const QString &localPrefix) override
    {
        bool useAppIdForOverride = true;
        QString path = metaPath(localPrefix, &useAppIdForOverride);
        if (path.isEmpty()) {
            qCWarning(cfLog, "Can't load meta file from local prefix: \"%s\"", qPrintable(localPrefix));
            return false;
        }

        QScopedPointer<QFile> meta(new QFile(path));

        struct _ScopedPointer {
            explicit _ScopedPointer(const QList<QIODevice*> &list)
                : m_list(list) {}
            ~_ScopedPointer() {qDeleteAll(m_list);}

            QList<QIODevice*> m_list;
        };
        _ScopedPointer overrides(loadOverrides(localPrefix, useAppIdForOverride));

        return load(meta.data(), overrides.m_list);
    }

    bool load(QIODevice *meta, const QList<QIODevice*> &overrides) override
    {
        {
            const QJsonDocument &doc = loadJsonFile(meta);
            if (!doc.isObject())
                return false;

            // 检查标识
            const QJsonObject &root = doc.object();
            if (!checkMagic(root, MAGIC_META)) {
                qCWarning(cfLog, "The meta magic does not match");
                return false;
            }

            // 检查版本兼容性
            const auto &v = parseVersion(root);
            if (!versionIsValid(v) || v.major > DConfigFile::supportedVersion().major) {
                qCWarning(cfLog, "The meta version number does not match, "
                                 "the file major version=%i, supported major version<=%i",
                          v.major, DConfigFile::supportedVersion().major);
                return false;
            }

            m_version = v;

            const auto &contents = root[QLatin1String("contents")].toObject();
            auto i = contents.constBegin();

            // 初始化原始值
            for (; i != contents.constEnd(); ++i) {
                values.update(i.key(), i.value().toObject().toVariantHash());
            }
        }

        // for override
        Q_FOREACH(auto override, overrides) {
            const QJsonDocument &doc = loadJsonFile(override);

            if (doc.isObject()) {
                const QJsonObject &root = doc.object();
                if (!checkMagic(root, MAGIC_OVERRIDE)) {
                    if (auto file = static_cast<QFile*>(override)) {
                        qCWarning(cfLog, "The override magic does not match, file: \"%s\", error message: \"%s\"",
                                  qPrintable(file->fileName()), qPrintable(file->errorString()));
                    } else {
                        qCWarning(cfLog, "The override magic does not match");
                    }
                    break; //TODO don't continue parse?
                }
                if (!checkVersion(root, m_version)) {
                    qCWarning(cfLog, "The override version number does not match");
                    break;
                }

                if (auto file = static_cast<QFile*>(override)) {
                    qCDebug(cfLog, "The override will be applied, file: \"%s\"", qPrintable(file->fileName()));
                }

                const auto &contents = root[QLatin1String("contents")].toObject();
                auto i = contents.constBegin();

                for (; i != contents.constEnd(); ++i) {
                    // 检查是否允许 override
                    if (values.flags(i.key()) & DConfigFile::NoOverride)
                        continue;

                    values.updateValue(i.key(), i.value());
                    values.updateSerial(i.key(), i.value());
                    values.updatePermissions(i.key(), i.value());
                }
            }
        }

        return true;
    }
    /*!
      \internal

        \brief 获得前缀为\a prefix目录的应用或公共库的所有覆盖机制目录,越后优先级越高
     */
    inline QStringList overrideDirs(const QString & prefix, bool useAppId) const {
        const QString &path2 = QString("%1/etc/dsg/configs/overrides/%2/%3")
                .arg(prefix, useAppId ? configKey.appId : QString(), configKey.fileName);

        QStringList paths;
        const QStringList &dataPaths = DStandardPaths::paths(DStandardPaths::DSG::DataDir);
        paths.reserve(dataPaths.size() + 1);
        for (auto path: dataPaths) {
            // reverse `DataDir`'s paths, previous `DataDir`'s value has high priority
            paths.prepend(QString("%1%2/configs/overrides/%3/%4")
                          .arg(prefix, path, useAppId ? configKey.appId : QString(), configKey.fileName));
        }
        // 在后面的优先级更高
        paths.append(path2);
        return paths;
    }

    inline QStringList allOverrideDirs(const bool useAppId, const QString &prefix) const override
    {
        QStringList dirs;
        // 只有当允许不使用 appid 时才能回退到通用目录
        if (!useAppId) {
            dirs << overrideDirs(prefix, false);
        }
        // 无论如何都先从带 appid 的目录下加载override文件
        // 在列表后面的更优先
        dirs << overrideDirs(prefix, true);
        return dirs;
    }
    /*!
      \internal

        \brief 获得所有遵守覆盖机制的文件流

        在override文件放置路径下按优先级查找覆盖文件,支持子目录查找机制,
        使用自然排序（如“a2”在“a11”之前）规则按文件名进行排序
     */
    QList<QIODevice *> loadOverrides(const QString &prefix, bool useAppId) const
    {
        auto filters = QDir::Files | QDir::NoDotAndDotDot | QDir::Readable;
        const QStringList nameFilters {"*" + FILE_SUFFIX};

        QStringList dirs = allOverrideDirs(useAppId, prefix);

        QList<QIODevice*> list;
        list.reserve(50);
        QCollator collator(QLocale::English);
        collator.setNumericMode(true);
        collator.setIgnorePunctuation(true);

        Q_FOREACH(const auto &dir, dirs) {
            const QDir base_dir(QDir::cleanPath(dir));

            if (!base_dir.exists())
                continue;

            QDir target_dir = base_dir;
            target_dir.setFilter(filters);
            target_dir.setNameFilters(nameFilters);

            if (!configKey.subpath.isEmpty())
                target_dir.cd(configKey.subpath.mid(1));

            do {
                qCDebug(cfLog, "load override file from: \"%s\"", qPrintable(target_dir.path()));

                QDirIterator iterator(target_dir);
                QList<QIODevice*> sublist;
                sublist.reserve(50);
                while(iterator.hasNext()) {
                    sublist.append(new QFile(iterator.next()));
                }

                // 从小到大排序
                std::sort(sublist.begin(), sublist.end(), [&collator](QIODevice *f1, QIODevice *f2){
                    if (collator.compare(static_cast<const QFile*>(f1)->fileName(),
                                         static_cast<const QFile*>(f2)->fileName()) < 0)
                        return true;
                    return false;
                });

                list = sublist + list;

                if (base_dir.path() == target_dir.path())
                    break;
            } while (target_dir.cdUp());
        }

        return list;
    }

    DConfigKey configKey;
    DConfigInfo values;
    DConfigFile::Version m_version = {0, 0};
    char padding [4] = {};
};

DConfigMetaImpl::DConfigMetaImpl(const DConfigKey &configKey)
    : DConfigMeta (),
     configKey(configKey)
{
}

DConfigMetaImpl::~DConfigMetaImpl()
{
}

/*!
    \class Dtk::Core::DConfigCache
    \inmodule dtkcore

    \brief 提供配置文件的用户和全局运行缓存访问接口.

*/

/*!
    \fn bool DConfigCache::load(const QString &localPrefix = QString()) = 0;
    \brief 解析缓存配置文件
    \return
*/

/*!
    \fn bool DConfigCache::save(const QString &localPrefix = QString(), QJsonDocument::JsonFormat format = QJsonDocument::Indented, bool sync = false) = 0;
    \brief 保存缓存的值到磁盘中
    \a localPrefix 为目录前缀
    \a format 保存格式
    \a sync 是否立即刷新
    \return
*/

/*!
    \fn bool DConfigCache::isGlobal() const = 0;
    \brief 是否是全局缓存
    \return
*/

/*!
    \fn void DConfigCache::remove(const QString &key) = 0;
    \brief 删除缓存中的配置项
    \a key 配置项名称
    \return
*/

/*!
    \fn QStringList DConfigCache::keyList() const = 0;
    \brief 返回配置内容的所有配置项
    \return
*/

/*!
    \fn bool DConfigCache::setValue(const QString &key, const QVariant &value, const int serial, const uint uid, const QString &callerAppid) = 0;
    \brief 设置缓存中的值
    \a key 配置项名称
    \a value 需要设置的值
    \a uid 设置时的用户id
    \a callerAppid 设置时的应用id
    \return 为true时表示重新设置了新值，false表示没有设置
*/

/*!
    \fn QVariant DConfigCache::value(const QString &key) const = 0;
    \brief 获取缓存中的值
    \a key 配置项名称
    \return
*/

/*!
    \fn int DConfigCache::serial(const QString &key) const = 0;
    \brief 返回配置项的单调递增值
    \a key 配置项名称
    \return -1为无效值，表明没有配置此项
*/

/*!
    \fn uint DConfigCache::uid() const = 0;
    \brief 用户标识，为全局缓存时，uid为非用户标识的特定值
    \return
*/

class Q_DECL_HIDDEN DConfigCacheImpl : public DConfigCache {
public:
    DConfigCacheImpl(const DConfigKey &configKey, const uint uid, bool global);
    virtual ~DConfigCacheImpl() override;

    // DConfigCache interface
public:
    inline virtual int serial(const QString &key) const override
    {
        return values.serial(key);
    }

    inline virtual uint uid() const override
    {
        return userid;
    }

    inline virtual QStringList keyList() const override
    {
        return values.keyList();
    }

    inline QString applicationCacheDir(const QString &prefix) const {
        const QString &homePath = DStandardPaths::homePath(userid);
        if (homePath.isEmpty()) {
            return QString();
        }
        const QString userHomeConfigDir = homePath + QStringLiteral("/.config/dsg/configs/");
        return prefix + userHomeConfigDir + "/" + configKey.appId;
    }

    inline QString cacheDir(const QString &basePath) {
        QDir dir(basePath + configKey.subpath);
        return dir.filePath(configKey.fileName + FILE_SUFFIX);
    }

    inline QString globalCacheDir(const QString &prefix) const {
        // TODO `DSG_APP_DATA` is not set and `appid` is not captured in `DStandardPaths::path`.
        QString appDataDir = DStandardPaths::path(DStandardPaths::DSG::AppData);
        if (appDataDir.isEmpty())
            appDataDir = QString("/var/dsg/appdata");

        return QString("%1/%2/configs/%3").arg(prefix, appDataDir, configKey.appId);
    }

    QString getCacheDir(const QString &localPrefix = QString())
    {
        if (isGlobal()) {
            return globalCacheDir(localPrefix);
        } else {
            return applicationCacheDir(localPrefix);
        }
    }

    bool load(const QString &localPrefix = QString()) override;

    bool isGlobal() const override
    {
        return global;
    }

    inline void remove(const QString &key) override
    {
        values.remove(key);
    }
    bool setValue(const QString &key, const QVariant &value, const int serial, const uint uid, const QString &appid) override
    {
        if (values.value(key) == value) {
            return false;
        }
        values.setValue(key, value);
        values.setSerial(key, serial);
        values.setTime(key, QDateTime::currentDateTime().toString(Qt::ISODate));
        values.setUser(key, uid);
        values.setAppId(key, appid.isEmpty() ? configKey.appId : appid);
        return true;
    }

    inline QVariant value(const QString &key) const override
    {
        return values.value(key);
    }

    bool save(const QString &localPrefix, QJsonDocument::JsonFormat format, bool sync) override;

    DConfigKey configKey;
    DConfigInfo values;
    uint userid;
    bool global;
    char padding [3] = {};
};

DConfigCacheImpl::DConfigCacheImpl(const DConfigKey &configKey, const uint uid, bool global)
    : DConfigCache(),
      configKey(configKey),
      userid(uid),
      global(global)
{
}

DConfigCacheImpl::~DConfigCacheImpl()
{
}

bool DConfigCacheImpl::load(const QString &localPrefix)
{
    // cache 文件要严格匹配 subpath
    const QString &dir = getCacheDir(localPrefix);
    if (dir.isEmpty()) {
        return true;
    }
    QScopedPointer<QFile> cache(loadFile(dir,
                                         configKey.subpath,
                                         configKey.fileName + FILE_SUFFIX,
                                         false));
    if (!cache) {
        return true;
    }

    const QJsonDocument &doc = loadJsonFile(cache.data());

    if (doc.isObject()) {
        const QJsonObject &root = doc.object();
        if (!checkMagic(root, MAGIC_CACHE))
            return false;
        if (!checkVersion(root, DConfigFile::supportedVersion()))
            return false;

        auto &&contents = root[QLatin1String("contents")].toObject();
        auto i = contents.constBegin();

        // 原样保存原始数据
        for (; i != contents.constEnd(); ++i) {
            values.update(i.key(), i.value().toObject().toVariantHash());
        }
    }
    return true;
}

bool DConfigCacheImpl::save(const QString &localPrefix, QJsonDocument::JsonFormat format, bool sync)
{
    const QString &dir = getCacheDir(localPrefix);
    if (dir.isEmpty()) {
        qCWarning(cfLog, "save Falied because home directory is not exist for the user[%d].", userid);
        return false;
    }
    QString path = cacheDir(dir);

    QFile cache(path);
    if (!QFile::exists(QFileInfo(cache.fileName()).path())) {
        QDir().mkpath(QFileInfo(cache.fileName()).path());
    }

    if (!cache.open(QIODevice::WriteOnly)) {
        qCWarning(cfLog, "save Falied on open file: \"%s\", error message: \"%s\"",
                  qPrintable(cache.fileName()), qPrintable(cache.errorString()));
        return false;
    }

    qCDebug(cfLog, "Save cache file \"%s\".", qPrintable(cache.fileName()));

    QJsonObject root;

    root[QLatin1String("magic")] = MAGIC_CACHE;
    const DConfigFile::Version version = DConfigFile::supportedVersion();
    root[QLatin1String("version")] = QString("%1.%2").arg(version.major)
            .arg(version.minor);

    root[QLatin1String("contents")] = values.content();
    QJsonDocument doc;
    doc.setObject(root);
    const QByteArray &json = doc.toJson(format);

    bool status = cache.write(json) == json.size();
    if (status && sync) {
        cache.flush();
    }
    return status;
}

class Q_DECL_HIDDEN DConfigFilePrivate : public DObjectPrivate {
public:
    DConfigFilePrivate(DConfigFile *qq, const QString &appId,
                       const QString &name, const QString &subpath)
        : DObjectPrivate(qq),
          configKey(appId, name ,subpath),
          configMeta(new DConfigMetaImpl(configKey))
    {
    }
    DConfigFilePrivate(DConfigFile *qq, const DConfigKey &configKey)
        : DObjectPrivate(qq),
          configKey(configKey),
          configMeta(new DConfigMetaImpl(configKey))
    {
    }

    ~DConfigFilePrivate() override;

    bool load(const QString &localPrefix)
    {
        bool status = configMeta->load(localPrefix);
        if (status) {
            // for cache
            status &= globalCache->load(localPrefix);
        }
        return status;
    }
    bool setValue(const QString &key, const QVariant &value,
                  DConfigCache *userCache, const QString &appid)
    {
        // 此处不要检查权限，在获取 value 时会检查
        if (auto cache = getCache(key, userCache)) {
            if (!value.isValid()) {
                cache->remove(key);
                return true;
            } else {
                const auto &metaValue = configMeta->value(key);
                // sample judgement to reduce a copy of convert.
                if (metaValue.type() == value.type())
                    return cache->setValue(key, value, configMeta->serial(key), cache->uid(), appid);

                // convert copy to meta's type, it promises `setValue` don't change meta's type.
                auto copy = value;
                if (!copy.convert(metaValue.userType())) {
                    qCWarning(cfLog) << "check type error, meta type is " << metaValue.type()
                                     << ", and now type is " << value.type();
                    return false;
                }

                return cache->setValue(key, copy, configMeta->serial(key), cache->uid(), appid);
            }
        }
        return false;
    }
    DConfigCache* getCache(const QString &key, DConfigCache *userCache) const
    {
        if(configMeta->flags(key).testFlag(DConfigFile::Global)) {
            return globalCache;
        }
        return userCache;
    }
    QVariant value(const QString &key, DConfigCache *userCache) const
    {
        // 检查权限
        if (configMeta->permissions(key) != DConfigFile::ReadOnly) {
            if (auto cache = getCache(key, userCache)) {
                if (DConfigInfo::checkSerial(configMeta->serial(key), cache->serial(key))) {
                    const QVariant &tmp = cache->value(key);
                    if (tmp.isValid())
                        return tmp;
                }
            }
        }

        return configMeta->value(key);
    }

    D_DECLARE_PUBLIC(DConfigFile)

private:
    DConfigCacheImpl* globalCache;
    DConfigKey configKey;
    DConfigMeta *configMeta;
};

DConfigFilePrivate::~DConfigFilePrivate()
{
    if (globalCache) {
        delete globalCache;
        globalCache = nullptr;
    }
    if (configMeta) {
        delete configMeta;
        configMeta = nullptr;
    }
}

/*!
    \brief 支持的版本
    \return
 */
constexpr DConfigFile::Version DConfigFile::supportedVersion()
{
    return DConfigFile::Version{1, 0};
}

/*!
    \brief 构造配置文件管理对象
    \a appId 应用程序唯一标识
    \a name 配置文件名
    \a subpath 子目录
 */
DConfigFile::DConfigFile(const QString &appId, const QString &name, const QString &subpath)
    : DObject(*new DConfigFilePrivate(this, appId, name, subpath))
{
    Q_ASSERT(!name.isEmpty());

    D_D(DConfigFile);
    d->globalCache = new DConfigCacheImpl(d->configKey, GlobalUID, true);
}

DConfigFile::DConfigFile(const DConfigFile &other)
    : DObject(*new DConfigFilePrivate(this, other.d_func()->configKey))
{
    D_D(DConfigFile);
    auto cache = new DConfigCacheImpl(d->configKey, GlobalUID, true);
    cache->values = other.d_func()->globalCache->values;
    d->globalCache = cache;
}

/*!
    \brief 解析配置文件
    \a localPrefix 为目录前缀
    \return
*/
bool DConfigFile::load(const QString &localPrefix)
{
    D_D(DConfigFile);
    return d->load(localPrefix);
}

/*!
    \brief 解析配置文件流
    \a meta 为原型流
    \a overrides 为覆盖机制查找的文件流
    \return
*/
bool DConfigFile::load(QIODevice *meta, const QList<QIODevice *> &overrides)
{
    return this->meta()->load(meta, overrides);
}

/*!
    \brief 保存缓存的值到磁盘中
    \a format 保存格式
    \a sync 是否立即刷新
    \return
*/
bool DConfigFile::save(const QString &localPrefix, QJsonDocument::JsonFormat format, bool sync) const
{
    D_DC(DConfigFile);

    bool ok = d->globalCache->save(localPrefix, format, sync);

    return ok;
}

/*!
 * \brief DConfigFile::value
 * \param key 配置项名称
 * \param uid 用户id，当key为全局项时，uid无效
 * \return
 */
QVariant DConfigFile::value(const QString &key, DConfigCache *userCache) const
{
    D_DC(DConfigFile);
    return d->value(key, userCache);
}

/*!
    \brief 设置缓存中的值
    \a key 配置项名称
    \a value 需要设置的值
    \a uid 设置时的用户id
    \a appid 设置时的应用id
    \return 为true时表示重新设置了新值，false表示没有设置
 */
bool DConfigFile::setValue(const QString &key, const QVariant &value, const QString &callerAppid, DConfigCache *userCache)
{
    D_D(DConfigFile);
    return d->setValue(key, value, userCache, callerAppid);
}

DConfigCache *DConfigFile::createUserCache(const uint uid)
{
    D_D(DConfigFile);
    return new DConfigCacheImpl(d->configKey, uid, false);
}


/*!
    \brief 返回全局缓存
    \return
 */
DConfigCache *DConfigFile::globalCache() const
{
    D_DC(DConfigFile);
    return d->globalCache;
}

/*!
    \brief 返回原型对象
    \return
 */
DConfigMeta *DConfigFile::meta()
{
    D_D(DConfigFile);
    return d->configMeta;
}

/*!
    \brief 检测配置文件是否有效
    \return
 */
bool DConfigFile::isValid() const
{
    D_DC(DConfigFile);
    return versionIsValid(d->configMeta->version());
}

DCORE_END_NAMESPACE
