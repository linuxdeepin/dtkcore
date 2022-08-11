// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPINYIN_H
#define DPINYIN_H

#include <dtkcore_global.h>

#include <QHash>

DCORE_BEGIN_NAMESPACE

QString LIBDTKCORESHARED_EXPORT Chinese2Pinyin(const QString& words);

DCORE_END_NAMESPACE

#endif // DPINYIN_H
