/*
 * Copyright (C) 2022 deepin Technology Co., Ltd.
 *
 * Author:     JiDeZhang <zhangjide@uniontech.com>
 *
 * Maintainer: JiDeZhang <zhangjide@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DSGAPPLICATION_H
#define DSGAPPLICATION_H

#include <dtkcore_global.h>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DSGApplication
{
public:
    static QByteArray id();
    static QByteArray getId(qint64 pid);
};

DCORE_END_NAMESPACE

#endif // DSGAPPLICATION_H
