// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define _GNU_SOURCE

#include "dsysinfo.h"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QThread>
#include <QFile>

#include <stdio.h>
#include <unistd.h> 
#include <cstring>

DCORE_USE_NAMESPACE

class StderrFilterThread : public QThread
{
public:
    explicit StderrFilterThread(int origStderr, int readFd, QObject *parent = nullptr)
        : QThread(parent), m_origStderr(origStderr), m_readFd(readFd) {}

protected:
    void run() override
    {
        FILE *readEnd = fdopen(m_readFd, "r");
        if (!readEnd) return;

        char *line = nullptr;
        size_t len = 0;
        while (getline(&line, &len, readEnd) != -1) {
            if (strstr(line, "WARNING: CPU random generator seem to be failing") ||
                strstr(line, "WARNING: RDRND generated:")) {
                continue; // 丢弃该行
            }
            write(m_origStderr, line, strlen(line));
        }
        free(line);
        fclose(readEnd);
    }

private:
    int m_origStderr;
    int m_readFd;
};

bool distributionInfoValid() {
    return QFile::exists(DSysInfo::distributionInfoPath());
}

void printDistributionOrgInfo(DSysInfo::OrgType type) {
    QString sectionName = DSysInfo::distributionInfoSectionName(type);
    printf("%s Name: %s\n", qPrintable(sectionName), qPrintable(DSysInfo::distributionOrgName(type)));
    printf("%s Logo (Normal size): %s\n", qPrintable(sectionName), qPrintable(DSysInfo::distributionOrgLogo(type)));
    printf("%s Website: %s\n", qPrintable(sectionName), qPrintable(DSysInfo::distributionOrgWebsite(type).second));
}

int main(int argc, char *argv[])
{
    // 设置过滤器
    int origStderr = dup(STDERR_FILENO);
    int pipefd[2];
    pipe(pipefd);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    StderrFilterThread *filterThread = new StderrFilterThread(origStderr, pipefd[0]);
    filterThread->start(); 

    QCoreApplication app(argc, argv);
    Q_UNUSED(app)

    QCommandLineParser parser;
    QCommandLineOption option_all("all", "Print All Information");
    QCommandLineOption option_deepin_type("deepin-type", " ");
    QCommandLineOption option_deepin_version("deepin-version", " ");
    QCommandLineOption option_deepin_edition("deepin-edition", " ");
    QCommandLineOption option_deepin_copyright("deepin-copyright", " ");
    QCommandLineOption option_product_type("product-type", " ");
    QCommandLineOption option_product_version("product-version", " ");
    QCommandLineOption option_computer_name("computer-name", "Computer Name");
    QCommandLineOption option_cpu_model("cpu-model", "CPU Model");
    QCommandLineOption option_installed_memory_size("installed-memory-size", "Installed Memory Size (GiB)");
    QCommandLineOption option_memory_size("memory-size", "Memory Size (GiB)");
    QCommandLineOption option_disk_size("disk-size", "Disk Size (GiB)");
    QCommandLineOption option_distribution_info("distribution-info", "Distribution information");
    QCommandLineOption option_distributer_info("distributer-info", "Distributer information");

    parser.addOptions({option_all, option_deepin_type, option_deepin_version, option_deepin_edition,
                       option_deepin_copyright, option_product_type, option_product_version,
                       option_computer_name, option_cpu_model, option_installed_memory_size, option_memory_size,
                       option_disk_size, option_distribution_info, option_distributer_info});
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    if (argc < 2)
        parser.showHelp();

    if (parser.isSet(option_all)) {
        printf("Computer Name: %s\n", qPrintable(DSysInfo::computerName()));
        printf("CPU Model: %s x %d\n", qPrintable(DSysInfo::cpuModelName()), QThread::idealThreadCount());
        printf("Installed Memory Size: %f GiB\n", DSysInfo::memoryInstalledSize() / 1024.0 / 1024 / 1024);
        printf("Memory Size: %f GiB\n", DSysInfo::memoryTotalSize() / 1024.0 / 1024 / 1024);
        printf("Disk Size: %f GiB\n", DSysInfo::systemDiskSize() / 1024.0 / 1024 / 1024);

        if (DSysInfo::isDeepin() && DSysInfo::isDDE()) {
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

        if (DSysInfo::isDeepin()) {
            printf("Uos Product Name: %s\n", qPrintable(DSysInfo::uosProductTypeName()));
            printf("Uos SystemName Name: %s\n", qPrintable(DSysInfo::uosSystemName()));
            printf("Uos Edition Name: %s\n", qPrintable(DSysInfo::uosEditionName()));
            printf("Uos SP Version: %s\n", qPrintable(DSysInfo::spVersion()));
            printf("Uos update Version: %s\n", qPrintable(DSysInfo::udpateVersion()));
            printf("Uos major Version: %s\n", qPrintable(DSysInfo::majorVersion()));
            printf("Uos minor Version: %s\n", qPrintable(DSysInfo::minorVersion()));
            printf("Uos build Version: %s\n", qPrintable(DSysInfo::buildVersion()));
        }
        if (distributionInfoValid()) {
            printDistributionOrgInfo(DSysInfo::Distribution);
            printDistributionOrgInfo(DSysInfo::Distributor);
        }
    } else {
        if (parser.isSet(option_deepin_type))
            printf("%s", qPrintable(DSysInfo::uosEditionName(QLocale::c())));
        else if (parser.isSet(option_deepin_version))
            printf("%s", qPrintable(DSysInfo::majorVersion()));
        else if (parser.isSet(option_deepin_edition))
            printf("%s", qPrintable(DSysInfo::deepinEdition()));
        else if (parser.isSet(option_deepin_copyright))
            printf("%s", qPrintable(DSysInfo::deepinCopyright()));
        else if (parser.isSet(option_product_type))
            printf("%s", qPrintable(DSysInfo::productTypeString()));
        else if (parser.isSet(option_product_version))
            printf("%s", qPrintable(DSysInfo::productVersion()));
        else if (parser.isSet(option_cpu_model))
            printf("%s x %d", qPrintable(DSysInfo::cpuModelName()), QThread::idealThreadCount());
        else if (parser.isSet(option_computer_name))
            printf("%s", qPrintable(DSysInfo::computerName()));
        else if (parser.isSet(option_installed_memory_size))
            printf("%f", DSysInfo::memoryInstalledSize() / 1024.0 / 1024 / 1024);
        else if (parser.isSet(option_memory_size))
            printf("%f", DSysInfo::memoryTotalSize() / 1024.0 / 1024 / 1024);
        else if (parser.isSet(option_disk_size))
            printf("%f", DSysInfo::systemDiskSize() / 1024.0 / 1024 / 1024);
        else if (parser.isSet(option_distribution_info)) {
            printDistributionOrgInfo(DSysInfo::Distribution);
        } else if (parser.isSet(option_distributer_info)) {
            printDistributionOrgInfo(DSysInfo::Distributor);
        }
    }

    // 清理
    dup2(origStderr, STDERR_FILENO);  // 恢复 stderr
    close(origStderr);

    filterThread->wait();             
    delete filterThread;

    return 0;
}
