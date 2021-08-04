/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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
#include "dsysinfo.h"
#include "ddesktopentry.h"

#include <QFile>
#include <QLocale>
#include <QStorageInfo>
#include <QProcess>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <QStandardPaths>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif

#ifndef OS_VERSION_TEST_FILE
#define OS_VERSION_FILE "/etc/os-version"
#else
#define OS_VERSION_FILE OS_VERSION_TEST_FILE
#endif

DCORE_BEGIN_NAMESPACE

class Q_DECL_HIDDEN DSysInfoPrivate
{
public:
    DSysInfoPrivate();

#ifdef Q_OS_LINUX
    void ensureDeepinInfo();
    bool ensureOsVersion();
    void ensureDistributionInfo();
    bool splitA_BC_DMode();
#endif
    void ensureReleaseInfo();
    void ensureComputerInfo();
    QMap<QString, QString> parseInfoFile(QFile &file);

#ifdef Q_OS_LINUX
    DSysInfo::DeepinType deepinType = DSysInfo::DeepinType(-1);
    QMap<QString, QString> deepinTypeMap; //Type Name with Language
    QString deepinVersion;
    QString deepinEdition;
    QString deepinCopyright;

    QString majorVersion;
    QString minorVersion;
    struct MinVersion {
        enum Type {
            A_BC_D, // 专业版
            X_Y_Z, // 家庭版
            A_B_C // 社区版
        };
        MinVersion()
            : A(0)
            , B(0)
            , BC(0)
            , C(0)
            , D(0)
            , X(0)
            , Y(0)
            , Z(0)
        {
        }

        uint A, B, BC, C, D; // A-BC-D
        uint X, Y, Z;
        Type type;
    };
    struct OSBuild {
        OSBuild():A(0), B(0), C(0), D(0), xyz(100){
        }
        uint A, B, C, D, E, xyz; // ABCDE.xyz
    };

    MinVersion minVersion;
    OSBuild osBuild;
#endif

    QScopedPointer<DDesktopEntry> distributionInfo;

    DSysInfo::ProductType productType = DSysInfo::ProductType(-1);
    QString prettyName;
    QString productTypeString;
    QString productVersion;

    QString computerName;
    QString cpuModelName;
    qint64 memoryAvailableSize = -1;
    qint64 memoryInstalledSize = -1;
    qint64 diskSize = 0;
};

DSysInfoPrivate::DSysInfoPrivate()
{

}

#ifdef Q_OS_LINUX
void DSysInfoPrivate::ensureDistributionInfo()
{
    if (distributionInfo)
        return;

    const QString distributionInfoFile(DSysInfo::distributionInfoPath());
    // Generic DDE distribution info
    distributionInfo.reset(new DDesktopEntry(distributionInfoFile));
}

bool DSysInfoPrivate::splitA_BC_DMode()
{
    // A-BC-D
    bool ok = false;
    uint minv = minorVersion.toUInt(&ok);
    if (ok) {
        minVersion.D = minv % 10;
    } else if (minorVersion.length() > 0) {
        const QString D = minorVersion.right(1);
        if (D.contains(QRegExp("[0-9A-Z]"))) {
            // 0-9...A-Z
            minVersion.D = 10 + static_cast<uint>(D.data()->toLatin1() - 'A');
        } else {
            qWarning() << "invalid minorVersion";
            minVersion.D = 0;
        }
    }
    uint minVer = minorVersion.left(3).toUInt();
    minVersion.BC = minVer % 100;
    minVer /= 100;
    minVersion.A = minVer % 10;
    minVersion.type = MinVersion::A_BC_D;
    return ok;
}

void DSysInfoPrivate::ensureDeepinInfo()
{
    if (static_cast<int>(deepinType) >= 0)
        return;

    QFile file("/etc/deepin-version");

    if (!file.open(QFile::ReadOnly)) {
        deepinType = DSysInfo::UnknownDeepin;

        return;
    }

    char buf[1024];
    int buf_length = 0;

    Q_FOREVER {
        buf_length = file.readLine(buf, sizeof(buf));

        if (buf_length < 0)
            break;

        const QByteArray line(buf, buf_length);
        const QByteArrayList &list = line.split('=');

        if (list.count() != 2) {
            continue;
        }

        const auto key_value = qMakePair(list.first().trimmed(), list.last().trimmed());

        if (key_value.first == "Version") {
            deepinVersion = key_value.second;
        } else if (line.startsWith("Type")) {
            if (key_value.first == "Type") {
                deepinTypeMap[QString()] = QString::fromLatin1(key_value.second);
            } else if (key_value.first.at(4) == '[' && key_value.first.at(key_value.first.size() - 1) == ']') {
                const QByteArray &language = key_value.first.mid(5, key_value.first.size() - 6);

                if (!language.isEmpty()) {
                    deepinTypeMap[QString::fromLatin1(language)] = QString::fromUtf8(key_value.second);
                }
            }
        } else if (key_value.first == "Edition") {
            deepinEdition = QString::fromUtf8(key_value.second);
        } else if (key_value.first == "Copyright") {
            deepinCopyright = QString::fromUtf8(key_value.second);
        }

        if (!deepinTypeMap.isEmpty() && !deepinEdition.isEmpty() && !deepinCopyright.isEmpty()) {
            break;
        }
    }

    file.close();

    const QString &deepin_type = deepinTypeMap[QString()];

    if (deepin_type.isEmpty()) {
        deepinType = DSysInfo::UnknownDeepin;
    } else if (deepin_type == "Desktop") {
        deepinType = DSysInfo::DeepinDesktop;
    } else if (deepin_type == "Professional") {
        deepinType = DSysInfo::DeepinProfessional;
    } else if (deepin_type == "Server") {
        deepinType = DSysInfo::DeepinServer;
    } else if (deepin_type == "Personal") {
        deepinType = DSysInfo::DeepinPersonal;
    } else {
        deepinType = DSysInfo::UnknownDeepin;
    }
}

bool DSysInfoPrivate::ensureOsVersion()
{
#ifndef OS_VERSION_TEST_FILE // 测试时总是重新读取文件
    if (osBuild.A > 0)
        return true;
#endif

    DDesktopEntry entry(OS_VERSION_FILE);
    bool ok = false;

    // 先获取版本信息
    // ABCDE.xyz
    QString osb = entry.stringValue("OsBuild", "Version");
    QStringList osbs = osb.split(".");
    Q_ASSERT(osbs.size() == 2 && osbs.value(0).size() == 5);
    uint left = osbs.value(0).trimmed().toUInt(&ok);
    Q_ASSERT(ok);
    if (ok) {
        osBuild.E = left % 10;
        left /= 10;
        osBuild.D = left % 10;
        left /= 10;
        osBuild.C = left % 10; // default C is 0
        left /= 10;
        osBuild.B = left % 10;
        left /= 10;
        osBuild.A = left % 10;
    }

    // xyz
    osBuild.xyz = osbs.value(1).trimmed().toUInt(&ok);

    majorVersion = entry.stringValue("MajorVersion", "Version");
    minorVersion = entry.stringValue("MinorVersion", "Version");

    switch (osBuild.D) {
    case 7: {
        // 家庭版使用“完整版本号编码-X.Y.Z”的形式
        const QStringList &versionList = minorVersion.split('.');
        if (versionList.isEmpty()) {
            // 如果读取失败直接返回为空
            qWarning() << "no minorVersion";
            return false;
        } else if (versionList.length() == 2) {
            // Z为0
            minVersion.X = versionList.first().toUInt();
            minVersion.Y = versionList.last().toUInt();
            minVersion.Z = 0;
        } else if (versionList.length() == 3) {
            // X.Y.Z都存在
            minVersion.X = versionList.at(0).toUInt();
            minVersion.Y = versionList.at(1).toUInt();
            minVersion.Z = versionList.at(2).toUInt();
        }
        minVersion.type = MinVersion::X_Y_Z;
    } break;

    case 3: {
        // 社区版使用“完整版本号编码-A.B.C”的形式
        bool a_bc_dMode = false;
        const QStringList &versionList = minorVersion.split('.');
        if (versionList.isEmpty()) {
            // 如果读取失败直接返回为空
            qWarning() << "no minorVersion";
            return false;
        } else if (versionList.length() == 1) {
            QString modeVersion = versionList.first();
            if (modeVersion.length() == 2) {
                //A.B.C模式且B C 为0
                minVersion.A = modeVersion.toUInt();
                minVersion.B = 0;
                minVersion.C = 0;
            } else {
                // A_BC_D模式
                splitA_BC_DMode();
                a_bc_dMode = true;
            }
        } else if (versionList.length() == 2) {
            // C为0
            minVersion.A = versionList.first().toUInt();
            minVersion.B = versionList.last().toUInt();
            minVersion.C = 0;
        } else if (versionList.length() == 3) {
            // A.B.C都存在
            minVersion.A = versionList.at(0).toUInt();
            minVersion.B = versionList.at(1).toUInt();
            minVersion.C = versionList.at(2).toUInt();
        }

        if (!a_bc_dMode)
            minVersion.type = MinVersion::A_B_C;
    } break;
    default: {
        // A-BC-D
        ok = splitA_BC_DMode();
    } break;
    }
    return ok;
}

static QString unquote(const QByteArray &value)
{
    if (value.at(0) == '"' || value.at(0) == '\'') {
        return QString::fromLatin1(value.mid(1, value.size() - 2));
    }

    return QString::fromLatin1(value);
}

static bool readEtcFile(DSysInfoPrivate *info, const char *filename,
                        const QByteArray &idKey, const QByteArray &versionKey, const QByteArray &prettyNameKey)
{

    QFile file(QString::fromLatin1(filename));

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    quint8 valid_data_count = 0;
    char buf[1024];

    while (valid_data_count < 3) {
        int buf_length = file.readLine(buf, sizeof(buf));

        if (buf_length < 0)
            break;

        const QByteArray line(buf, buf_length - 1);

        if (info->productTypeString.isEmpty() && line.startsWith(idKey)) {
            const QByteArray value(line.constData() + idKey.size());
            info->productTypeString = unquote(value);
            ++valid_data_count;
            continue;
        }

        if (info->prettyName.isEmpty() && line.startsWith(prettyNameKey)) {
            const QByteArray value(line.constData() + prettyNameKey.size());
            info->prettyName = unquote(value);
            ++valid_data_count;
            continue;
        }

        if (info->productVersion.isEmpty() && line.startsWith(versionKey)) {
            const QByteArray value(line.constData() + versionKey.size());
            info->productVersion = unquote(value);
            ++valid_data_count;
            continue;
        }
    }

    file.close();

    return valid_data_count != 0;
}

static bool readOsRelease(DSysInfoPrivate *info)
{
    if (!readEtcFile(info, "/etc/os-release", "ID=", "VERSION_ID=", "PRETTY_NAME="))
        return readEtcFile(info, "/usr/lib/os-release", "ID=", "VERSION_ID=", "PRETTY_NAME=");

    return true;
}

static bool readLsbRelease(DSysInfoPrivate *info)
{
    return readEtcFile(info, "/etc/lsb-release", "DISTRIB_ID=", "DISTRIB_RELEASE=", "DISTRIB_DESCRIPTION=");
}
#endif

void DSysInfoPrivate::ensureReleaseInfo()
{
    if (productType >= 0) {
        return;
    }

#ifdef Q_OS_LINUX
    readOsRelease(this);
    readLsbRelease(this);

    if (productTypeString.isEmpty()) {
        productType = DSysInfo::UnknownType;
    } else {
        switch (productTypeString.at(0).unicode()) {
        case 'd':
        case 'D':
            if (productTypeString.compare("deepin", Qt::CaseInsensitive) == 0) {
                productType = DSysInfo::Deepin;
            } else if (productTypeString.compare("debian", Qt::CaseInsensitive) == 0) {
                productType = DSysInfo::Debian;
            }
            break;
        case 'a':
        case 'A':
            if (productTypeString.compare("arch", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::ArchLinux;
            break;
        case 'c':
        case 'C':
            if (productTypeString.compare("centos", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::CentOS;
            break;
        case 'f':
        case 'F':
            if (productTypeString.compare("fedora", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::Fedora;
            break;
        case 'l':
        case 'L':
            if (productTypeString.compare("linuxmint", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::LinuxMint;
            break;
        case 'm':
        case 'M':
            if (productTypeString.compare("manjaro", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::Manjaro;
            break;
        case 'o':
        case 'O':
            if (productTypeString.compare("opensuse", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::openSUSE;
            break;
        case 's':
        case 'S':
            if (productTypeString.compare("sailfishos", Qt::CaseInsensitive) == 0)
                productType = DSysInfo::SailfishOS;
            break;
        case 'u':
        case 'U':
            if (productTypeString.compare("ubuntu", Qt::CaseInsensitive) == 0) {
                productType = DSysInfo::Ubuntu;
            } else if (productTypeString.compare("uos", Qt::CaseInsensitive) == 0 || productTypeString.compare("UnionTech OS", Qt::CaseInsensitive) == 0) {
                productType = DSysInfo::Uos;
            }
            break;
        default:
            productType = DSysInfo::UnknownType;
            break;
        }
    }
#endif
}

void DSysInfoPrivate::ensureComputerInfo()
{
#ifdef Q_OS_LINUX

#endif
}

QMap<QString, QString> DSysInfoPrivate::parseInfoFile(QFile &file)
{
    char buf[1024];
    qint64 lineLength = 0;
    QMap<QString, QString> map;
    do {
        lineLength = file.readLine(buf, sizeof(buf));
        QString s(buf);
        if (s.contains(':')) {
            QStringList list = s.split(':');
            if (list.size() == 2) {
                map.insert(list.first().trimmed(), list.back().trimmed());
            }
        }
    } while (lineLength >= 0);
    return map;
}

Q_GLOBAL_STATIC(DSysInfoPrivate, siGlobal)

QString DSysInfo::operatingSystemName()
{
    siGlobal->ensureReleaseInfo();

    return siGlobal->prettyName;
}

#ifdef Q_OS_LINUX
/*!
 * \brief Check current distro is Deepin or not.
 * \note Uos will also return true.
 */
bool DSysInfo::isDeepin()
{
    siGlobal->ensureReleaseInfo();

    return productType() == Deepin || productType() == Uos;
}

bool DSysInfo::isDDE()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinType != UnknownDeepin;
}

DSysInfo::DeepinType DSysInfo::deepinType()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinType;
}

QString DSysInfo::deepinTypeDisplayName(const QLocale &locale)
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinTypeMap.value(locale.name(), siGlobal->deepinTypeMap.value(QString()));
}

QString DSysInfo::deepinVersion()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinVersion;
}

QString DSysInfo::deepinEdition()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinEdition;
}

QString DSysInfo::deepinCopyright()
{
    siGlobal->ensureDeepinInfo();

    return siGlobal->deepinCopyright;
}

/*!
 * \~chinese \brief DSysInfo::osType 系统类型
 * \~chinese \row 显示系统类型【1：桌面】【2：服务器】【3：专用设备】
 * \~chinese \note 根据 osBuild.B 判断
 */
DSysInfo::UosType DSysInfo::uosType()
{
    siGlobal->ensureOsVersion();

    UosType ost = UosTypeUnknown;
    if ((siGlobal->osBuild.B > UosTypeUnknown && siGlobal->osBuild.B < UosTypeCount)) {
        ost = static_cast<UosType>(siGlobal->osBuild.B);
    }

    return ost;
}

/*!
 * \~chinese \brief DSysInfo::osEditionType 版本类型
 * \~chinese \row 显示版本类型 专业版/个人版/社区版...
 * \~chinese \note 根据 osBuild.B && osBuild.D
 */
DSysInfo::UosEdition DSysInfo::uosEditionType()
{
    siGlobal->ensureOsVersion();
    UosEdition ospt = UosEditionUnknown;
    if (siGlobal->osBuild.B == UosDesktop) {
        switch (siGlobal->osBuild.D) {
        case 1:
            return UosProfessional;
        case 2:
        case 7:
            // 新版本家庭版(7)与旧版本个人版(2)同为Home 不修改旧有逻辑的情况下新增7保证对旧版的适配
            return UosHome;
        case 3:
            return UosCommunity;
        case 4:
            return UosMilitary;
        case 5:
            return UosDeviceEdition;
        case 6:
            return UosEducation;
        default:
            break;
        }
    } else if (siGlobal->osBuild.B == UosServer) {
        switch (siGlobal->osBuild.D) {
        case 1:
            return UosEnterprise;
        case 2:
            return UosEnterpriseC;
        case 3:
            return UosEuler;
        case 4:
            return UosMilitaryS;
        case 5:
            return UosDeviceEdition;
        default:
            break;
        }
    } else if (siGlobal->osBuild.B == UosDevice){
        ospt = UosEnterprise; // os-version 1.4 if B==Device then et=Enterprise
    }

    return ospt;
}

/*!
 * \~chinese \brief DSysInfo::osArch 架构信息（使用一个字节的二进制位，从低位到高位）
 * \~chinese \row 【0x8 sw64】【0x4 mips64】【0x2 arm64】【0x1 amd64】
 */
DSysInfo::UosArch DSysInfo::uosArch()
{
    siGlobal->ensureOsVersion();

    return static_cast<UosArch>(siGlobal->osBuild.E);
}

static QString getUosVersionValue(const QString &key, const QLocale &locale)
{
    DDesktopEntry entry(OS_VERSION_FILE);
    QString localKey = QString("%1[%2]").arg(key, locale.name());

    return entry.stringValue(localKey, "Version", entry.stringValue(key, "Version"));
}

/*!
 * \~chinese \brief DSysInfo::osProductTypeName 版本名称
 * \~chinese \row ProductType[xx] 项对应的值, 如果找不到对应语言的默认使用 ProductType的值(Desktop/Server/Device)
 * \~chinese \param locale 当前系统语言
 */
QString DSysInfo::uosProductTypeName(const QLocale &locale)
{
    return getUosVersionValue("ProductType", locale);
}

/*!
 * \~chinese \brief DSysInfo::osSystemName 版本名称
 * \~chinese \row SystemName[xx] 项对应的值, 如果找不到对应语言的默认使用 SystemName 的值 Uniontech OS
 * \~chinese \param locale 当前系统语言
 */
QString DSysInfo::uosSystemName(const QLocale &locale)
{
    return getUosVersionValue("SystemName", locale);
}

/*!
 * \~chinese \brief DSysInfo::osEditionName 版本名称
 * \~chinese \row EditionName[xx] 项对应的值, 如果找不到对应语言的默认使用 EditionName 的值(Professional/Home/Community...)
 * \~chinese \param locale 当前系统语言
 */
QString DSysInfo::uosEditionName(const QLocale &locale)
{
    return getUosVersionValue("EditionName", locale);
}

/*!
 * \~chinese \brief DSysInfo::spVersion 阶段版本名称
 * \~chinese \row 小版本号 A-BC-D 中 BC、 A.B.C 中的 B
 * \~chinese \row 返回 SP1-SPxx， 如果正式版返回空
 * \~chinese \row X.Y.Z模式下暂不支持返回此版本号
 * \~chinese \note minVersion.BC == 00：正式版本    minVersion.BC | minVersion.B == 01-99：SP1….SP99
 */
QString DSysInfo::spVersion()
{
    siGlobal->ensureOsVersion();
    switch (siGlobal->minVersion.type) {
    case DSysInfoPrivate::MinVersion::A_BC_D: {
        if (siGlobal->minVersion.BC > 0) {
            return QString("SP%1").arg(siGlobal->minVersion.BC);
        } else {
            return QString(); // 00 正式版
        }
    }

    case DSysInfoPrivate::MinVersion::A_B_C: {
        if (siGlobal->minVersion.B > 0) {
            return QStringLiteral("SP%1").arg(siGlobal->minVersion.B);
        } else {
            return {};
        }
    }

    case DSysInfoPrivate::MinVersion::X_Y_Z:
        qWarning() << "Getting the SP version in this mode is not supported.";
        return {};
    }
}

/*!
 * \~chinese \brief DSysInfo::udpateVersion 更新版本名称
 * \~chinese \row 小版本号 A-BC-D 中 D、A.B.C 模式中的 C
 * \~chinese \row 返回 update1… update9， 如果正式版返回空
 * \~chinese \row X.Y.Z模式下暂不支持返回此版本号
 * \~chinese \note minVersion.D == 0：正式版本    minVersion.D | minVersion.C == 1-9：update1… update9,updateA...updateZ
 */
QString DSysInfo::udpateVersion()
{
    siGlobal->ensureOsVersion();
    switch (siGlobal->minVersion.type) {
    case DSysInfoPrivate::MinVersion::A_BC_D: {
        if (siGlobal->minVersion.D > 0) {
            uint uv = siGlobal->minVersion.D;
            if (uv < 10) {
                return QString("update%1").arg(uv);
            } else if (uv < 36) {
                return QString("update").append(QChar(uv - 10 + 'A'));
            } else {
                qWarning() << "invalid update versoin";
                break;
            }
        } else {
            break; // 0 正式版
        }
    }

    case DSysInfoPrivate::MinVersion::A_B_C: {
        if (siGlobal->minVersion.C > 0) {
            return QStringLiteral("update%1").arg(siGlobal->minVersion.C);
        } else {
            break;
        }
    }

    case DSysInfoPrivate::MinVersion::X_Y_Z:
        qWarning() << "Getting the update version in this mode is not supported.";
        break;
    }

    return {};
}

/*!
 * \~chinese \brief DSysInfo::majorVersion 主版本号
 * \~chinese \row 主版本号 【20】【23】【25】【26】【29】【30】
 * \~chinese \note 返回 MajorVersion 的值
 */
QString DSysInfo::majorVersion()
{
    siGlobal->ensureOsVersion();
    return siGlobal->majorVersion;
}

/*!
 * \~chinese \brief DSysInfo::minorVersion 小版本号
 * \~chinese \row 【ABCD】 ·[0-9]{4}
 * \~chinese \row【A.B.C】 或者【X.Y.Z】
 * \~chinese \note 返回 MinorVersion 的值
 */
QString DSysInfo::minorVersion()
{
    siGlobal->ensureOsVersion();
    return siGlobal->minorVersion;
}

/*!
 * \~chinese \brief DSysInfo::buildVersion 小版本号
 * \~chinese \row 系统镜像批次号，按时间顺序（不可回退）从100-999递增
 * \~chinese \note 返回 osBuild.xyz 的值
 */
QString DSysInfo::buildVersion()
{
    siGlobal->ensureOsVersion();
    return QString::number(siGlobal->osBuild.xyz);
}
#endif

QString DSysInfo::deepinDistributionInfoPath()
{
    return distributionInfoPath();
}

QString DSysInfo::distributionInfoPath()
{
#ifdef Q_OS_LINUX
    return "/usr/share/deepin/distribution.info";
#else
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).filePath("deepin-distribution.info");
#endif // Q_OS_LINUX
}

QString DSysInfo::distributionInfoSectionName(DSysInfo::OrgType type)
{
    switch (type) {
    case Distribution:
        return "Distribution";
    case Distributor:
        return "Distributor";
    case Manufacturer:
        return "Manufacturer";
    }

    return QString();
}

/*!
 * \return the organization name.
 *
 * use \l type as Distribution to get the name of current deepin distribution itself.
 *
 * \sa deepinDistributionInfoPath()
 */
QString DSysInfo::distributionOrgName(DSysInfo::OrgType type, const QLocale &locale)
{
#ifdef Q_OS_LINUX
    siGlobal->ensureDistributionInfo();
#endif

    QString fallback = type == Distribution ? QStringLiteral("Deepin") : QString();

    return siGlobal->distributionInfo->localizedValue("Name", locale, distributionInfoSectionName(type), fallback);
}

QString DSysInfo::deepinDistributorName()
{
    return distributionOrgName(Distributor);
}

/*!
 * \return the organization website name and url.
 *
 * use \l type as Distribution to get the name of current deepin distribution itself.
 *
 * \sa deepinDistributionInfoPath()
 */
QPair<QString, QString> DSysInfo::distributionOrgWebsite(DSysInfo::OrgType type)
{
#ifdef Q_OS_LINUX
    siGlobal->ensureDistributionInfo();
#endif

    QString fallbackSiteName = type == Distribution ? QStringLiteral("www.deepin.org") : QString();
    QString fallbackSiteUrl = type == Distribution ? QStringLiteral("https://www.deepin.org") : QString();

    return {
        siGlobal->distributionInfo->stringValue("WebsiteName", distributionInfoSectionName(type), fallbackSiteName),
        siGlobal->distributionInfo->stringValue("Website", distributionInfoSectionName(type), fallbackSiteUrl),
    };
}

QPair<QString, QString> DSysInfo::deepinDistributorWebsite()
{
    return distributionOrgWebsite(Distributor);
}

/*!
 * \return the obtained organization logo path, or the given \l fallback one if there are no such logo.
 *
 * use \l type as Distribution to get the logo of current deepin distribution itself.
 *
 * \sa deepinDistributionInfoPath()
 */
QString DSysInfo::distributionOrgLogo(DSysInfo::OrgType orgType, DSysInfo::LogoType type, const QString &fallback)
{
    DDesktopEntry distributionInfo(distributionInfoPath());
    QString orgSectionName = distributionInfoSectionName(orgType);

    switch (type) {
    case Normal:
        return distributionInfo.stringValue("Logo", orgSectionName, fallback);
    case Light:
        return distributionInfo.stringValue("LogoLight", orgSectionName, fallback);
    case Symbolic:
        return distributionInfo.stringValue("LogoSymbolic", orgSectionName, fallback);
    case Transparent:
        return distributionInfo.stringValue("LogoTransparent", orgSectionName, fallback);
    }

    return QString();
}

QString DSysInfo::deepinDistributorLogo(DSysInfo::LogoType type, const QString &fallback)
{
    return distributionOrgLogo(Distributor, type, fallback);
}

DSysInfo::ProductType DSysInfo::productType()
{
    siGlobal->ensureReleaseInfo();

    return siGlobal->productType;
}

QString DSysInfo::productTypeString()
{
    siGlobal->ensureReleaseInfo();

    return siGlobal->productTypeString;
}

QString DSysInfo::productVersion()
{
    siGlobal->ensureReleaseInfo();

    return siGlobal->productVersion;
}

/*!
 * \brief Check if current edition is a community edition
 *
 * Developer can use this way to check if we need enable or disable features
 * for community or enterprise edition.
 *
 * Current rule:
 *  - Professional, Server, Personal edition (DeepinType) will be treat as Enterprise edition.
 *  - Uos (ProductType) will be treat as Enterprise edition.
 *
 * \return true if it's on a community edition distro/installation
 */
bool DSysInfo::isCommunityEdition()
{
#ifdef Q_OS_LINUX
    DeepinType type = deepinType();
    QList<DeepinType> enterpriseTypes {
        DeepinProfessional, DeepinServer, DeepinPersonal
    };

    if (enterpriseTypes.contains(type)) {
        return false;
    }

    if (productType() == Uos) {
        return false;
    }
#endif // Q_OS_LINUX

    return true;
}

QString DSysInfo::computerName()
{
#ifdef Q_OS_LINUX
    struct utsname u;
    if (uname(&u) == 0)
        siGlobal->computerName = QString::fromLatin1(u.nodename);

    return siGlobal->computerName;
#endif
    return QString();
}

QString DSysInfo::cpuModelName()
{
#ifdef Q_OS_LINUX
    static QFile file("/proc/cpuinfo");

    if (file.open(QFile::ReadOnly)) {
        QMap<QString, QString> map = siGlobal->parseInfoFile(file);
        if (map.contains("Processor")) {
            // arm-cpuinfo hw_kirin-cpuinfo
            siGlobal->cpuModelName = map.value("Processor");
        } else if (map.contains("model name")) {
            // cpuinfo
            siGlobal->cpuModelName = map.value("model name");
        } else if (map.contains("cpu model")) {
            // loonson3-cpuinfo sw-cpuinfo
            siGlobal->cpuModelName = map.value("cpu model");
        }

        file.close();
    }
    return siGlobal->cpuModelName;
#endif
    return QString();
}

/*!
 * \return the installed memory size
 */
qint64 DSysInfo::memoryInstalledSize()
{
#ifdef Q_OS_LINUX
    // Getting Memory Installed Size
    // TODO: way to not dept on lshw?
    if (!QStandardPaths::findExecutable("lshw").isEmpty()) {
        QProcess lshw;

        lshw.start("lshw", {"-c", "memory", "-json", "-sanitize"}, QIODevice::ReadOnly);

        if (!lshw.waitForFinished()) {
            return -1;
        }

        const QByteArray &lshwInfoJson = lshw.readAllStandardOutput();
        QJsonArray lshwResultArray = QJsonDocument::fromJson(lshwInfoJson).array();
        if (!lshwResultArray.isEmpty()) {
            QJsonValue memoryHwInfo = lshwResultArray.first();
            QString id = memoryHwInfo.toObject().value("id").toString();
            Q_ASSERT(id == "memory");
            siGlobal->memoryInstalledSize = memoryHwInfo.toObject().value("size").toDouble(); // TODO: check "units" is "bytes" ?
        }
    }

    return siGlobal->memoryInstalledSize;
#endif
    return -1;
}

/*!
 * \return the total available to use memory size
 */
qint64 DSysInfo::memoryTotalSize()
{
#ifdef Q_OS_LINUX
    siGlobal->memoryAvailableSize = get_phys_pages() * sysconf(_SC_PAGESIZE);
    return siGlobal->memoryAvailableSize;
#endif
    return -1;
}

qint64 DSysInfo::systemDiskSize()
{
#ifdef Q_OS_LINUX
    // Getting Disk Size
    const QString &deviceName = QStorageInfo::root().device();
    QProcess lsblk;

    lsblk.start("lsblk", {"-Jlpb", "-oNAME,KNAME,PKNAME,SIZE"}, QIODevice::ReadOnly);

    if (!lsblk.waitForFinished()) {
        return -1;
    }

    const QByteArray &diskStatusJson = lsblk.readAllStandardOutput();
    QJsonDocument diskStatus = QJsonDocument::fromJson(diskStatusJson);
    QJsonValue diskStatusJsonValue = diskStatus.object().value("blockdevices");
    QMap<QString, QPair<QString, qulonglong>> deviceParentAndSizeMap;

    if (!diskStatusJsonValue.isUndefined()) {
        QJsonArray diskStatusArray = diskStatusJsonValue.toArray();
        QString keyName;

        for (const QJsonValue oneValue : diskStatusArray) {
            QString name = oneValue.toObject().value("name").toString();
            QString kname = oneValue.toObject().value("kname").toString();
            QString pkname = oneValue.toObject().value("pkname").toString();
            qulonglong size = oneValue.toObject().value("size").toVariant().toULongLong();

            if (keyName.isNull() && deviceName == name) {
                keyName = kname;
            }

            deviceParentAndSizeMap[kname] = QPair<QString, qulonglong>(pkname, size);
        }

        while (!deviceParentAndSizeMap[keyName].first.isNull()) {
            keyName = deviceParentAndSizeMap[keyName].first;
        }

        siGlobal->diskSize = deviceParentAndSizeMap[keyName].second;
    }

    return siGlobal->diskSize;

#endif

    return -1;
}

DCORE_END_NAMESPACE
