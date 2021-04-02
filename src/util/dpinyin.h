/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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
 *
 * pinyin.dict from https://github.com/flyerhzm/chinese_pinyin
 */

#ifndef DPINYIN_H
#define DPINYIN_H

#include <dtkcore_global.h>

#include <QHash>

DCORE_BEGIN_NAMESPACE

QString LIBDTKCORESHARED_EXPORT Chinese2Pinyin(const QString& words);

DCORE_END_NAMESPACE

#endif // DPINYIN_H
