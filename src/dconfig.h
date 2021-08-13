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

class DConfigPrivate;
class LIBDTKCORESHARED_EXPORT DConfig : public QObject, public DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DConfig)

    Q_PROPERTY(QStringList keyList READ keyList FINAL)

public:
    explicit DConfig(const QString &name, const QString &subpath = QString(),
                     QObject *parent = nullptr);

    bool load();

    QStringList keyList() const;

    bool isValid() const;
    QVariant value(const QString &key, const QVariant &fallback = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);

Q_SIGNALS:
    void valueChanged(const QString &key);
};

DCORE_END_NAMESPACE

#endif // DCONFIG_H
