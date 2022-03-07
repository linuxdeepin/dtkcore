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
#ifndef DCONFIG_H
#define DCONFIG_H

#include <dtkcore_global.h>
#include <DObject>

#include <QObject>
#include <QVariant>

DCORE_BEGIN_NAMESPACE
class DConfigBackend {
public:
    virtual ~DConfigBackend();
    virtual bool isValid() const = 0;
    virtual bool load(const QString &/*appId*/) = 0;
    virtual QStringList keyList() const = 0;
    virtual QVariant value(const QString &/*key*/, const QVariant &/*fallback*/) const = 0;
    virtual void setValue(const QString &/*key*/, const QVariant &/*value*/) = 0;
    virtual void reset(const QString &key) { setValue(key, QVariant());}
    virtual QString name() const {return QString("");}
};

class DConfigPrivate;
class LIBDTKCORESHARED_EXPORT DConfig : public QObject, public DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DConfig)

    Q_PROPERTY(QStringList keyList READ keyList FINAL)

public:
    explicit DConfig(const QString &name, const QString &subpath = QString(),
                     QObject *parent = nullptr);

    explicit DConfig(DConfigBackend *backend, const QString &name, const QString &subpath = QString(),
                     QObject *parent = nullptr);

    static DConfig *create(const QString &appId, const QString &name, const QString &subpath = QString(),
                           QObject *parent = nullptr);
    static DConfig *create(DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath = QString(),
                                  QObject *parent = nullptr);

    QString backendName() const;

    QStringList keyList() const;

    bool isValid() const;
    QVariant value(const QString &key, const QVariant &fallback = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);
    void reset(const QString &key);

    QString name() const;
    QString subpath() const;

Q_SIGNALS:
    void valueChanged(const QString &key);

private:
    explicit DConfig(DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath,
                     QObject *parent = nullptr);
};

DCORE_END_NAMESPACE

#endif // DCONFIG_H
