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
#ifndef DCONFIGFILE_H
#define DCONFIGFILE_H

#include <dtkcore_global.h>
#include <DObject>
#include <QStringList>
#include <QFlags>
#include <QVariant>
#include <QLocale>
#include <QJsonDocument>
#include <QDebug>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

DCORE_BEGIN_NAMESPACE

class DConfigFilePrivate;
class LIBDTKCORESHARED_EXPORT DConfigFile : public DObject
{
    D_DECLARE_PRIVATE(DConfigFile)
public:
    enum Flag {
        NoOverride = 1 << 0,
        Global = 1 << 1
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum Permissions {
        ReadOnly,
        ReadWrite
    };

    struct Version {
        quint16 major;
        quint16 minor;
    };

    static constexpr Version supportedVersion();

    explicit DConfigFile(const QString &appId, const QString &name,
                         const QString &subpath = QString());

    bool load(const QString &localPrefix = QString());
    bool load(QIODevice *meta, QIODevice *userCache,
              QIODevice *globalCache, QList<QIODevice*> overrides);
    bool save(QJsonDocument::JsonFormat format = QJsonDocument::Indented,
              bool sync = false) const;
    bool save(QIODevice *cache, bool global,
              QJsonDocument::JsonFormat format = QJsonDocument::Indented) const;

    bool isValid() const;

    Version version() const;
    void setVersion(quint16 major, quint16 minor);

    QStringList keyList() const;
    Flags flags(const QString &key) const;
    Permissions permissions(const QString &key) const;

    QVariant value(const QString &key, const QVariant &fallback = QVariant()) const;
    void setValue(const QString &key, const QVariant &value,
                  const QString &userName, const QString &appid);

    QString displayName(const QString &key, QLocale::Language lang);
    QString description(const QString &key, QLocale::Language lang);

protected:
    friend QDebug operator<<(QDebug, const DConfigFile &);
};

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const DConfigFile &);
#endif

Q_DECLARE_OPERATORS_FOR_FLAGS(DConfigFile::Flags)

DCORE_END_NAMESPACE

#endif // DCONFIGFILE_H
