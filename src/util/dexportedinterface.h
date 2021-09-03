/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Chris Xiong <chirs241097@gmail.com>
 *
 * Maintainer: Chris Xiong <chirs241097@gmail.com>
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

#ifndef DEXPORTEDINTERFACE_H
#define DEXPORTEDINTERFACE_H

#include <dtkcore_global.h>
#include <DObject>

#include <QObject>

#include <functional>

DCORE_BEGIN_NAMESPACE

namespace DUtil {
class DExportedInterfacePrivate;
class LIBDTKCORESHARED_EXPORT DExportedInterface : public QObject, public DObject
{
    Q_OBJECT
public:
    explicit DExportedInterface(QObject *parent = nullptr);
    ~DExportedInterface();

    void registerAction(const QString &action, const QString &description, const std::function<QVariant(QString)> handler = nullptr);
    virtual QVariant invoke(const QString &action, const QString &parameters) const;
private:
    D_DECLARE_PRIVATE(DExportedInterface)
};
}

DCORE_END_NAMESPACE

#endif
