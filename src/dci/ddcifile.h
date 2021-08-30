/*
 * Copyright (C) 2021 UnionTech Technology Co., Ltd.
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <DObject>
#include <dtkcore_global.h>

#include <QStringList>

QT_BEGIN_HEADER
class QIODevice;
QT_END_NAMESPACE

DCORE_BEGIN_NAMESPACE

class DDciFilePrivate;
class LIBDTKCORESHARED_EXPORT DDciFile : public DObject
{
    D_DECLARE_PRIVATE(DDciFile)
public:
    enum FileType {
        UnknowFile,
        File = 1,
        Directory = 2
    };

    DDciFile();
    explicit DDciFile(const QString &fileName);
    explicit DDciFile(const QByteArray &data);

    bool isValid() const;
    QString lastErrorString() const;

    bool writeToFile(const QString &fileName) const;
    bool writeToDevice(QIODevice *device) const;
    QByteArray toData() const;

    static constexpr int metadataSizeV1();

    // for reader
    QStringList list(const QString &dir, bool onlyFileName = false) const;
    int childrenCount(const QString &dir) const;
    bool exists(const QString &filePath) const;
    FileType type(const QString &filePath) const;
    QByteArray dataRef(const QString &filePath) const;

    // for writer
    bool mkdir(const QString &filePath);
    bool writeFile(const QString &filePath, const QByteArray &data, bool override = false);
    bool remove(const QString &filePath);
    bool rename(const QString &filePath, const QString &newFilePath, bool override = false);
    bool copy(const QString &from, const QString &to);
};

DCORE_END_NAMESPACE
