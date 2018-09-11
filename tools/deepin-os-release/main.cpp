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
#include "dsysinfo.h"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QThread>

#include <stdio.h>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app)

    QCommandLineParser parser;
    QCommandLineOption option_all("all");
    QCommandLineOption option_deepin_type("deepin-type");
    QCommandLineOption option_deepin_version("deepin-version");
    QCommandLineOption option_deepin_edition("deepin-edition");
    QCommandLineOption option_deepin_copyright("deepin-copyright");
    QCommandLineOption option_product_type("product-type");
    QCommandLineOption option_product_version("product-version");
    QCommandLineOption option_computer_name("computer-name");
    QCommandLineOption option_cpu_model("cpu-model");
    QCommandLineOption optioin_memory_size("memory-size");
    QCommandLineOption optioin_disk_size("disk-size");

    parser.addOptions({option_all, option_deepin_type, option_deepin_version, option_deepin_edition,
                       option_deepin_copyright, option_product_type, option_product_version,
                       option_computer_name, option_cpu_model, optioin_memory_size, optioin_disk_size});
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    if (parser.isSet(option_all)) {
        printf("Computer Name: %s\n", qPrintable(DSysInfo::computerName()));
        printf("CPU Model: %s x %d\n", qPrintable(DSysInfo::cpuModelName()), QThread::idealThreadCount());
        printf("Memory Size: %f GiB\n", DSysInfo::memoryTotalSize() / 1024.0 / 1024 / 1024);
        printf("Disk Size: %f GiB\n", DSysInfo::systemDiskSize() / 1024.0 / 1024 / 1024);

        if (DSysInfo::isDDE()) {
            printf("Deepin Type: %s\n", qPrintable(DSysInfo::deepinTypeDisplayName()));
            printf("Deepin Version: %s\n", qPrintable(DSysInfo::deepinVersion()));

            if (!DSysInfo::deepinEdition().isEmpty())
                printf("Deepin Edition: %s\n", qPrintable(DSysInfo::deepinEdition()));

            if (!DSysInfo::deepinCopyright().isEmpty())
                printf("Deepin Copyright: %s\n", qPrintable(DSysInfo::deepinCopyright()));
        }

        printf("Operating System Name: %s\n", qPrintable(DSysInfo::operatingSystemName()));
        printf("Product Type: %s\n", qPrintable(DSysInfo::productTypeString()));
        printf("Product Version: %s\n", qPrintable(DSysInfo::productVersion()));
    } else {
        if (parser.isSet(option_deepin_type))
            printf("%s", qPrintable(DSysInfo::deepinTypeDisplayName(QLocale::c())));
        else if (parser.isSet(option_deepin_version))
            printf("%s", qPrintable(DSysInfo::deepinVersion()));
        else if (parser.isSet(option_deepin_edition))
            printf("%s", qPrintable(DSysInfo::deepinEdition()));
        else if (parser.isSet(option_deepin_copyright))
            printf("%s", qPrintable(DSysInfo::deepinCopyright()));
        else if (parser.isSet(option_product_type))
            printf("%s", qPrintable(DSysInfo::productTypeString()));
        else if (parser.isSet(option_product_version))
            printf("%s", qPrintable(DSysInfo::productVersion()));
    }

    return 0;
}
