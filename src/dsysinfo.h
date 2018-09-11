/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DSYSINFO_H
#define DSYSINFO_H

#include <dtkcore_global.h>

#include <QLocale>

DCORE_BEGIN_NAMESPACE

class DSysInfoPrivate;
class DSysInfo
{
public:
    enum ProductType {
        UnknownType = 0,
        Deepin,
        ArchLinux,
        CentOS,
        Debian,
        Fedora,
        LinuxMint,
        Manjaro,
        openSUSE,
        SailfishOS,
        Ubuntu
    };

    enum DeepinType {
        UnknownDeepin = 0,
        DeepinDesktop,
        DeepinProfessional
    };

#ifdef Q_OS_LINUX
    static bool isDeepin();
    static bool isDDE();
    static DeepinType deepinType();
    static QString deepinTypeDisplayName(const QLocale &locale = QLocale::system());
    static QString deepinVersion();
    static QString deepinEdition();
    static QString deepinCopyright();
#endif

    static QString operatingSystemName();
    static ProductType productType();
    static QString productTypeString();
    static QString productVersion();

    static QString computerName();
    static QString cpuModelName();
    static qint64 memoryTotalSize();
    static qint64 systemDiskSize();
};

DCORE_END_NAMESPACE

#endif // DSYSINFO_H
