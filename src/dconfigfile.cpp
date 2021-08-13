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

// https://gitlabwh.uniontech.com/wuhan/se/deepin-specifications/-/issues/3

DCORE_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(cfLog, "dtk.dsg.config"
#ifndef QT_DEBUG
, QtInfoMsg
#endif
);

#define FILE_SUFFIX QLatin1String(".json")

inline QFile *loadFile(const QString &baseDir, const QString &subpath, const QString &name,
                       bool canFallbackUp = true) {
    qCDebug(cfLog, "load json file from base dir: \"%s\"", qPrintable(baseDir));
    qCDebug(cfLog, "subpath = \"%s\"", qPrintable(subpath));
    qCDebug(cfLog, "file name = \"%s\"", qPrintable(name));

    const QDir base_dir(baseDir);
    QDir target_dir = base_dir;

    if (!subpath.isEmpty())
        target_dir.cd(subpath.mid(1));

    do {
        qCDebug(cfLog, "load json file from: \"%s\"", qPrintable(target_dir.path()));

        QFile *file = new QFile(target_dir.filePath(name));
        if (!file->exists()) {
            delete file;
        } else {
            return file;
        }

        if (base_dir == target_dir)
            break;
    } while (canFallbackUp && target_dir.cdUp());

    return nullptr;
}

class Q_DECL_HIDDEN DConfigFilePrivate : public DObjectPrivate {
public:
    DConfigFilePrivate(DConfigFile *qq, const QString &appId,
                       const QString &name, const QString &subpath);

    static QJsonDocument load(QIODevice *data);

    inline static QString applicationMetaDir(const QString &prefix) {
        const QByteArray &app_root = qgetenv("APP_ROOT");
        return QString("%1%2/configs")
                .arg(prefix, QString::fromLocal8Bit(app_root));
    }

    inline static QString genericMetaDir(const QString & prefix) {
        return prefix + DStandardPaths::filePath(DStandardPaths::DSG::DataDir,
                                                         QString("configs"));
    }

    inline QStringList overrideDirs(const QString & prefix, bool useAppId) const {
        const QString &path2 = QString("%1/etc/dsg/configs/overrides/%2/%3")
                .arg(prefix, useAppId ? appId : QString(), name);

        const QString &path1 = QString("%1%2/configs/overrides/%3/%4")
                .arg(prefix, DStandardPaths::path(DStandardPaths::DSG::DataDir),
                     useAppId ? appId : QString(), name);

        // 在后面的优先级更高
        return {path1, path2};
    }

    inline QString applicationCacheDir(const QString & prefix) const {
        return prefix + DStandardPaths::filePath(DStandardPaths::XDG::ConfigHome, appId);
    }

    inline QString globalCacheDir(const QString & prefix) const {
        return prefix + DStandardPaths::filePath(DStandardPaths::DSG::AppData,
                                                         QString("configs"));
    }

    QList<QIODevice*> loadOverrides(QString prefix, bool useAppId = true) const;
    bool load(QIODevice *meta, QIODevice *userCache, QIODevice *globalCache, const QList<QIODevice *> &overrides);
    void setValue(const QString &key, const QVariant &value, const QString &userName, const QString &appid);
    void applyCache(QIODevice *cache, QHash<QString, QVariantHash> &data);

    inline DConfigFile::Flags flags(const QString &key) const;
    inline DConfigFile::Permissions permissions(const QString &key) const;
    inline QVariant value(const QString &key) const;
    inline QString displayName(const QString &key, QLocale::Language lang) const;
    inline QString description(const QString &key, QLocale::Language lang) const;

    D_DECLARE_PUBLIC(DConfigFile)

    QString appId;
    QString name;
    QString path;
    DConfigFile::Version version = {0, 0};
    // 原始的值（包含 override 数据）
    QHash<QString, QVariantHash> values;
    struct {
        // 修改过用户级别的值
        QHash<QString, QVariantHash> user;
        // 修改过的 global 级别的值
        QHash<QString, QVariantHash> global;
    } caches;
};

DConfigFilePrivate::DConfigFilePrivate(DConfigFile *qq, const QString &appId,
                                       const QString &name, const QString &subpath)
    : DObjectPrivate(qq)
    , appId(appId)
    , name(name)
    , path(subpath)
{

}

QJsonDocument DConfigFilePrivate::load(QIODevice *data)
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
    int major = items.first().toUInt(&ok);

    if (!ok)
        return version;

    int minor = items.last().toUInt(&ok);

    if (!ok)
        return version;

    version.major = major;
    version.minor = minor;

    return version;
}

#define MAGIC_META QLatin1String("dsg.config.meta")
#define MAGIC_OVERRIDE QLatin1String("dsg.config.override")
#define MAGIC_CACHE QLatin1String("dsg.config.cache")

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

QList<QIODevice *> DConfigFilePrivate::loadOverrides(QString prefix, bool useAppId) const
{
    auto filters = QDir::Files | QDir::NoDotAndDotDot | QDir::Readable;
    const QStringList nameFilters {"*" + FILE_SUFFIX};

    QStringList dirs;
    // 只有当允许不使用 appid 时才能回退到通用目录
    if (!useAppId) {
        dirs << overrideDirs(prefix, false);
    }
    // 无论如何都先从带 appid 的目录下加载override文件
    // 在列表后面的更优先
    dirs << overrideDirs(prefix, true);

    QList<QIODevice*> list;
    list.reserve(50);
    QCollator collator(QLocale::system());
    collator.setNumericMode(true);
    collator.setIgnorePunctuation(true);

    Q_FOREACH(const auto &path, dirs) {
        const QDir base_dir(path);

        if (!base_dir.exists())
            continue;

        QDir target_dir = base_dir;
        target_dir.setFilter(filters);
        target_dir.setNameFilters(nameFilters);

        if (!path.isEmpty())
            target_dir.cd(path.mid(1));

        do {
            qCDebug(cfLog, "load override file from: \"%s\"", qPrintable(target_dir.path()));

            QDirIterator iterator(target_dir);
            QList<QIODevice*> sublist;
            sublist.reserve(50);

            while (iterator.hasNext()) {
                QFile *file = new QFile(iterator.next());
                // 插入排序
                for (int i = 0; i < sublist.size(); ++i) {
                    auto f = static_cast<const QFile*>(sublist.at(i));
                    // 从小到大排序
                    if (collator.compare(f->fileName(), file->fileName()) < 0) {
                        sublist.insert(i, file);
                        file = nullptr;
                        break;
                    }
                }

                if (file)
                    sublist.append(file);
            }

            list = sublist + list;

            if (base_dir == target_dir)
                break;
        } while (target_dir.cdUp());
    }

    return list;
}

bool DConfigFilePrivate::load(QIODevice *meta, QIODevice *userCache,
                              QIODevice *globalCache, const QList<QIODevice*> &overrides)
{
    if (name.isEmpty())
        return false;

    const QJsonDocument &doc = load(meta);
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

    version = v;
    // 清空旧数据
    values.clear();
    caches.user.clear();
    caches.global.clear();

    const auto &contents = root[QLatin1String("contents")].toObject();
    auto i = contents.constBegin();

    // 初始化原始值
    for (; i != contents.constEnd(); ++i) {
        values[i.key()] = i.value().toObject().toVariantHash();
    }

    // for override
    Q_FOREACH(auto override, overrides) {
        const QJsonDocument &doc = load(override);

        if (doc.isObject()) {
            const QJsonObject &root = doc.object();
            if (!checkMagic(root, MAGIC_OVERRIDE)) {
                qCWarning(cfLog, "The override magic does not match");
                break;
            }
            if (!checkVersion(root, version)) {
                qCWarning(cfLog, "The override version number does not match");
                break;
            }

            const auto &contents = root[QLatin1String("contents")].toObject();
            auto i = contents.constBegin();

            for (; i != contents.constEnd(); ++i) {
                // 检查是否允许 override
                if (flags(i.key()) & DConfigFile::NoOverride)
                    continue;

                auto &value = values[i.key()];
                overrideValue(QLatin1String("value"), i.value(), value);
                overrideValue(QLatin1String("serial"), i.value(), value);
                overrideValue(QLatin1String("permissions"), i.value(), value);
            }
        }
    }

    // for cache
    if (globalCache)
        applyCache(globalCache, caches.global);
    if (userCache)
        applyCache(userCache, caches.global);

    return true;
}

void DConfigFilePrivate::setValue(const QString &key, const QVariant &value,
                                  const QString &userName, const QString &appid)
{
    // 此处不要检查权限，在获取 value 时会检查
    auto &data = flags(key).testFlag(DConfigFile::Global) ? caches.global : caches.user;

    if (!value.isValid()) {
        data.remove(key);
    } else {
        data[key][QLatin1String("value")] = value;
        data[key][QLatin1String("serial")] = values[key][QLatin1String("serial")];
        data[key][QLatin1String("time")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        data[key][QLatin1String("user")] = userName;
        data[key][QLatin1String("appid")] = appid;
    }
}

void DConfigFilePrivate::applyCache(QIODevice *cache, QHash<QString, QVariantHash> &data)
{
    const QJsonDocument &doc = load(cache);

    if (doc.isObject()) {
        const QJsonObject &root = doc.object();
        if (!checkMagic(root, MAGIC_CACHE))
            return;
        if (!checkVersion(root, version))
            return;

        auto &&contents = root[QLatin1String("contents")].toObject();
        auto i = contents.constBegin();

        // 原样保存原始数据
        for (; i != contents.constEnd(); ++i) {
            data[i.key()] = i.value().toObject().toVariantHash();
        }
    }
}

DConfigFile::Flags DConfigFilePrivate::flags(const QString &key) const
{
    DConfigFile::Flags flags = {};
    const auto &value = values[key][QLatin1String("flags")];
    Q_FOREACH(const QString &flag, value.toStringList()) {
        if (flag == QLatin1String("nooverride")) {
            flags &= DConfigFile::NoOverride;
        } else if (flag == QLatin1String("global")) {
            flags &= DConfigFile::Global;
        }
    }

    return flags;
}

DConfigFile::Permissions DConfigFilePrivate::permissions(const QString &key) const
{
    DConfigFile::Permissions p = DConfigFile::ReadOnly;
    const auto &value = values[key][QLatin1String("permissions")].toString();
    if (value == QLatin1String("readwrite"))
        p = DConfigFile::ReadWrite;

    return p;
}

inline static bool checkSerial(const QVariantHash &value, const QVariantHash &request) {
    bool ok = false;
    int serial1 = request[QLatin1String("serial")].toInt(&ok);
    if (!ok)
        return true;
    int serial2 = value[QLatin1String("serial")].toInt(&ok);

    return ok && serial2 == serial1;
}

QVariant DConfigFilePrivate::value(const QString &key) const
{
    // 检查权限
    auto &data = flags(key).testFlag(DConfigFile::Global) ? caches.global : caches.user;
    if (permissions(key) != DConfigFile::ReadOnly
            && checkSerial(values[key], data[key])) {
        const auto &value = data[key][QLatin1String("value")];

        if (value.isValid())
            return value;
    }

    return values[key][QLatin1String("value")];
}

QString DConfigFilePrivate::displayName(const QString &key, QLocale::Language lang) const
{
    if (lang == QLocale::AnyLanguage)
        return values[key][QLatin1String("name")].toString();

    return values[key].value(QString("name[%1]")
                             .arg(QLocale::languageToString(lang))).toString();
}

QString DConfigFilePrivate::description(const QString &key, QLocale::Language lang) const
{
    if (lang == QLocale::AnyLanguage)
        return values[key][QLatin1String("description")].toString();

    return values[key].value(QString("description[%1]")
                             .arg(QLocale::languageToString(lang))).toString();
}

constexpr DConfigFile::Version DConfigFile::supportedVersion()
{
    return Version {1, 0};
}

DConfigFile::DConfigFile(const QString &appId, const QString &name, const QString &subpath)
    : DObject(*new DConfigFilePrivate(this, appId, name, subpath))
{
    Q_ASSERT(!name.isEmpty());
}

DConfigFile::Version DConfigFile::version() const
{
    D_DC(DConfigFile);
    return d->version;
}

void DConfigFile::setVersion(quint16 major, quint16 minor)
{
    if (!isValid())
        return;

    D_D(DConfigFile);
    d->version.major = major;
    d->version.minor = minor;
}

QStringList DConfigFile::keyList() const
{
    D_DC(DConfigFile);
    return d->values.keys();
}

DConfigFile::Flags DConfigFile::flags(const QString &key) const
{
    D_DC(DConfigFile);
    return d->flags(key);
}

DConfigFile::Permissions DConfigFile::permissions(const QString &key) const
{
    D_DC(DConfigFile);
    return d->permissions(key);
}

bool DConfigFile::load(const QString &localPrefix)
{
    D_D(DConfigFile);

    // 记录 meta 文件的类别，只有当 meta 文件是公共类型的文件时
    // 才允许从不带 appid 的路径下加载 override 文件
    bool useAppIdForOverride = true;
    QScopedPointer<QFile> meta;
    {
        auto file = loadFile(d->applicationMetaDir(localPrefix), d->path, d->name + FILE_SUFFIX);
        if (!file) {
            useAppIdForOverride = false;
            file = loadFile(d->genericMetaDir(localPrefix), d->path, d->name + FILE_SUFFIX);
        }
        meta.reset(file);
    }

    if (!meta)
        return false;

    struct _ScopedPointer {
        _ScopedPointer(const QList<QIODevice*> &list)
            : m_list(list) {}
        ~_ScopedPointer() {qDeleteAll(m_list);}

        QList<QIODevice*> m_list;
    };
    _ScopedPointer overrides(d->loadOverrides(localPrefix, useAppIdForOverride));
    // cache 文件要严格匹配 subpath
    QScopedPointer<QFile> userCache(loadFile(d->applicationCacheDir(localPrefix),
                                             d->path, d->name + FILE_SUFFIX, false));
    QScopedPointer<QFile> globalCache(loadFile(d->globalCacheDir(localPrefix),
                                               d->path, d->name + FILE_SUFFIX, false));

    return d->load(meta.data(), userCache.data(),
                   globalCache.data(), overrides.m_list);
}

bool DConfigFile::load(QIODevice *meta, QIODevice *userCache,
                       QIODevice *globalCache, QList<QIODevice*> overrides)
{
    Q_ASSERT(meta);
    D_D(DConfigFile);
    return d->load(meta, userCache, globalCache, overrides);
}

inline QString cacheDir(const DConfigFilePrivate *cf, const QString &basePath) {
    QDir dir(basePath + cf->path);
    return dir.filePath(cf->name + FILE_SUFFIX);
}

bool DConfigFile::save(QJsonDocument::JsonFormat format, bool sync) const
{
    D_DC(DConfigFile);

    QFile user(cacheDir(d, d->applicationCacheDir(QString())));
    QFile global(cacheDir(d, d->globalCacheDir(QString())));

    bool ok = true;
    if (user.open(QIODevice::WriteOnly)) {
        ok = ok && save(&user, false, format);
    }

    if (global.open(QIODevice::WriteOnly)) {
        ok = ok && save(&global, false, format);
    }

    if (sync) {
        user.flush();
        global.flush();
    }

    return ok;
}

bool DConfigFile::save(QIODevice *cache, bool global, QJsonDocument::JsonFormat format) const
{
    D_DC(DConfigFile);

    QJsonDocument doc;
    QJsonObject root;

    root[QLatin1String("magic")] = MAGIC_CACHE;
    root[QLatin1String("version")] = QString("%1.%2").arg(d->version.major)
                                                     .arg(d->version.minor);

    QJsonObject contents;
    const auto &data = global ? d->caches.global : d->caches.user;
    for (auto i = data.constBegin(); i != data.constEnd(); ++i) {
        contents[i.key()] = QJsonObject::fromVariantHash(i.value());
    }

    root[QLatin1String("contents")] = contents;
    doc.setObject(root);
    const QByteArray &json = doc.toJson(format);

    return cache->write(json) == json.size();
}

bool DConfigFile::isValid() const
{
    D_DC(DConfigFile);
    return versionIsValid(d->version);
}

QVariant DConfigFile::value(const QString &key, const QVariant &fallback) const
{
    D_DC(DConfigFile);
    const QVariant &v = d->value(key);
    return v.isValid() ? v : fallback;
}

void DConfigFile::setValue(const QString &key, const QVariant &value,
                           const QString &userName, const QString &appid)
{
    D_D(DConfigFile);

    // 检查权限
    if (d->permissions(key) == DConfigFile::ReadOnly)
        return;

    d->setValue(key, value, userName, appid);
}

QString DConfigFile::displayName(const QString &key, QLocale::Language lang)
{
    D_DC(DConfigFile);
    return d->displayName(key, lang);
}

QString DConfigFile::description(const QString &key, QLocale::Language lang)
{
    D_DC(DConfigFile);
    return d->description(key, lang);
}

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug dbg, const DConfigFile &config)
{
    dbg << config.d_func()->values;
    return dbg;
}

#endif

DCORE_END_NAMESPACE
