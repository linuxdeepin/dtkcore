/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "dpinyin.h"

#include <QFile>
#include <QTextStream>

DCORE_BEGIN_NAMESPACE

static QHash<uint, QString> dict = {};
const char kDictFile[] = ":/dpinyin/resources/dpinyin.dict";

static void InitDict() {
    if (!dict.isEmpty()) {
        return;
    }

    dict.reserve(25333);

    QFile file(kDictFile);

    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray content = file.readAll();

    file.close();

    QTextStream stream(&content, QIODevice::ReadOnly);

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QStringList items = line.split(QChar(':'));

        if (items.size() == 2) {
            dict.insert(items[0].toInt(nullptr, 16), items[1]);
        }
    }
}

QString Chinese2Pinyin(const QString &words)
{
    InitDict();

    QString result;

    for (int i = 0; i < words.length(); ++i) {
        const uint key = words.at(i).unicode();
        auto find_result = dict.find(key);

        if (find_result != dict.end()) {
            result.append(find_result.value());
        } else {
            result.append(words.at(i));
        }
    }

    return result;
}

DCORE_END_NAMESPACE
