/*
 * Copyright (C) 2015 ~ 2017 Deepin Technology Co., Ltd.
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

#ifndef DOBJECT_H
#define DOBJECT_H

#include <QScopedPointer>

#include <dtkcore_global.h>

DCORE_BEGIN_NAMESPACE

#define D_DECLARE_PRIVATE(Class) Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_d_ptr),Class)
#define D_DECLARE_PUBLIC(Class) Q_DECLARE_PUBLIC(Class)
#define D_D(Class) Q_D(Class)
#define D_Q(Class) Q_Q(Class)
#define D_DC(Class) Q_D(const Class)
#define D_QC(Class) Q_Q(const Class)
#define D_PRIVATE_SLOT(Func) Q_PRIVATE_SLOT(d_func(), Func)

class DObjectPrivate;

class LIBDTKCORESHARED_EXPORT DObject
{
protected:
    DObject(DObject *parent = nullptr);

    DObject(DObjectPrivate &dd, DObject *parent = nullptr);

    virtual ~DObject();

    QScopedPointer<DObjectPrivate> d_d_ptr;

    Q_DISABLE_COPY(DObject)
    D_DECLARE_PRIVATE(DObject)
};

DCORE_END_NAMESPACE

#endif // DOBJECT_H
