/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include "ut_dutil.h"

#include <QThread>
#include <QStandardPaths>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDir>
#include <DDesktopEntry>

#include "log/LogManager.h"
#include "filesystem/dpathbuf.h"
#include "ut_singleton.h"
#include "util/dtimeunitformatter.h"
#include "util/ddisksizeformatter.h"
#include "util/ddbussender.h"
#include "settings/dsettings.h"
#include "settings/dsettingsgroup.h"
#include "settings/dsettingsoption.h"
#include "dsysinfo.h"

DCORE_USE_NAMESPACE

void ut_DUtil::SetUpTestCase()
{
    //qDebug() << "*****************" << __FUNCTION__;
}

void ut_DUtil::TearDownTestCase()
{
    //qDebug() << "*****************" << __FUNCTION__;
}

void ut_DUtil::SetUp()
{
    QDir dir("/tmp/etc/");
    if (!dir.exists())
        dir.mkdir("/tmp/etc/");
}
void ut_DUtil::TearDown()
{
    QDir dir("/tmp/etc/");
    if (dir.exists())
        dir.remove("/tmp/etc/");
}

TEST_F(ut_DUtil, testLogPath)
{
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName("deepin-test-dtk");

    DPathBuf logPath(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());

#ifdef Q_OS_OSX
    logPath = logPath / "Library" / "Caches" / "deepin" / "deepin-test-dtk" / "deepin-test-dtk.log";
#else
    logPath = logPath / ".cache" / "deepin" / "deepin-test-dtk" / "deepin-test-dtk.log";
#endif

    ASSERT_EQ(DLogManager::getlogFilePath(), logPath.toString());
}

TEST_F(ut_DUtil, testPathChange)
{
    DPathBuf root("/");

    auto usr = root / "./usr";
    ASSERT_EQ(QDir(usr.toString()).absolutePath(), QDir::toNativeSeparators("/usr"));

    root /= "root";
    ASSERT_EQ(root.toString(), QDir::toNativeSeparators("/root"));

    root /= "../usr";
    ASSERT_EQ(root.toString(), usr.toString());
}

TEST_F(ut_DUtil, testDSingleton)
{
    auto threadA = new QThread;
    auto testerA = new MultiSingletonTester;
    QObject::connect(threadA, &QThread::started, testerA, &MultiSingletonTester::run);
    QObject::connect(threadA, &QThread::finished, testerA, [=]() {
        threadA->deleteLater();
        testerA->deleteLater();
    });
    testerA->moveToThread(threadA);

    auto threadB = new QThread;
    auto testerB = new MultiSingletonTester;
    testerB->moveToThread(threadB);
    QObject::connect(threadB, &QThread::started, testerB, &MultiSingletonTester::run);
    QObject::connect(threadB, &QThread::finished, testerB, [=]() {
        threadB->deleteLater();
        testerB->deleteLater();
    });

    threadA->start();
    threadB->start();

    QThread::sleep(5);
}

TEST_F(ut_DUtil, testTimeFormatter)
{
    const DTimeUnitFormatter timeFormatter;

    // 3600 seconds == 1 hour
    const auto r0 = timeFormatter.format(3600, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(r0.first, 1) && r0.second == DTimeUnitFormatter::Hour);

    // 86400 seconds == 1 day
    const auto r1 = timeFormatter.format(86400, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(r1.first, 1) && r1.second == DTimeUnitFormatter::Day);

    // 129600 seconds == 1.5 day
    const auto r3 = timeFormatter.format(129600, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(1.5, r3.first) && r3.second == DTimeUnitFormatter::Day);

    // 1.5 day == 36 hours
    const auto r4 = timeFormatter.formatAs(1.5, DTimeUnitFormatter::Day, DTimeUnitFormatter::Hour);
    ASSERT_TRUE(qFuzzyCompare(r4, 36));
}

TEST_F(ut_DUtil, testTimeFormatterList)
{
    const DTimeUnitFormatter timeFormatter;

    // 135120.5 Minutes == 93 days + 20 hours + 30 seconds
    const auto r = timeFormatter.formatAsUnitList(135120.5, DTimeUnitFormatter::Minute);
    ASSERT_TRUE(qFuzzyCompare(r[0].first, 93) && r[0].second == DTimeUnitFormatter::Day);
    ASSERT_TRUE(qFuzzyCompare(r[1].first, 20) && r[1].second == DTimeUnitFormatter::Hour);
    ASSERT_TRUE(qFuzzyCompare(r[2].first, 30) && r[2].second == DTimeUnitFormatter::Seconds);
}

TEST_F(ut_DUtil, testDiskFormatter)
{
    const DDiskSizeFormatter diskFormatter1000 = DDiskSizeFormatter();

    // 1000 K == 1 M
    const auto i0 = diskFormatter1000.format(1000, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(i0.first, 1) && i0.second == DDiskSizeFormatter::M);

    // 1000 K == 1000000 B
    const auto i1 = diskFormatter1000.formatAs(1000, DDiskSizeFormatter::K, DDiskSizeFormatter::B);
    ASSERT_TRUE(qFuzzyCompare(i1, 1000000));
}

TEST_F(ut_DUtil, testDiskFormatterList)
{
    const DDiskSizeFormatter diskFormatter = DDiskSizeFormatter();

    // 1351223412.1234 KB == 1 TB + 351 GB + 223 MB + 412 KB + 123.4 B
    const auto r = diskFormatter.formatAsUnitList(1351223412.1234, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(r[0].first, 1) && r[0].second == DDiskSizeFormatter::T);
    ASSERT_TRUE(qFuzzyCompare(r[1].first, 351) && r[1].second == DDiskSizeFormatter::G);
    ASSERT_TRUE(qFuzzyCompare(r[2].first, 223) && r[2].second == DDiskSizeFormatter::M);
    ASSERT_TRUE(qFuzzyCompare(r[3].first, 412) && r[3].second == DDiskSizeFormatter::K);

    // TODO: test failed
    //    Q_ASSERT(r[4].first == 123.4 && r[4].second == DiskSizeFormatter::B);
}

TEST_F(ut_DUtil, testDiskFormatter1024)
{
    const DDiskSizeFormatter diskFormatter = DDiskSizeFormatter().rate(1024);

    // 1024 K == 1 M
    const auto d0 = diskFormatter.format(1024, DDiskSizeFormatter::K);
    ASSERT_TRUE(qFuzzyCompare(d0.first, 1) && d0.second == DDiskSizeFormatter::M);

    // 100000000000 B == 93.13225746154785 G
    const auto d1 = diskFormatter.format(100000000000, DDiskSizeFormatter::B);
    ASSERT_TRUE(qFuzzyCompare(93.13225746154785, d1.first) && d1.second == DDiskSizeFormatter::G);

    // 100000000000 B == 0.09094947017729282 T
    const auto d2 = diskFormatter.formatAs(100000000000, DDiskSizeFormatter::B, DDiskSizeFormatter::T);
    ASSERT_TRUE(qFuzzyCompare(0.09094947017729282, d2));
}

TEST_F(ut_DUtil, testDBusSender)
{
    // basic method call
    DDBusSender()
        .service("com.deepin.dde.ControlCenter")
        .interface("com.deepin.dde.ControlCenter")
        .path("/com/deepin/dde/ControlCenter")
        .method("ShowPage")
        .arg(QString("update"))
        .arg(QString("available-updates"))
        .call();

    // property set
    QDBusPendingReply<> r1 = DDBusSender()
                                 .service("com.deepin.dde.daemon.Dock")
                                 .interface("com.deepin.dde.daemon.Dock")
                                 .path("/com/deepin/dde/daemon/Dock")
                                 .property("DisplayMode")
                                 .set(1); // set to efficient mode

    // property get
    QDBusPendingReply<QVariant> r2 = DDBusSender()
                                         .service("com.deepin.dde.daemon.Dock")
                                         .interface("com.deepin.dde.daemon.Dock")
                                         .path("/com/deepin/dde/daemon/Dock")
                                         .property("DisplayMode")
                                         .get(); // read mode

    if (!r2.isError() && !r1.isError()) {
        ASSERT_TRUE(r2.value().toInt() == 1);
    }

    // complex type property get
    QDBusPendingReply<QVariant> r3 = DDBusSender()
                                         .service("com.deepin.dde.ControlCenter")
                                         .interface("com.deepin.dde.ControlCenter")
                                         .path("/com/deepin/dde/ControlCenter")
                                         .property("Rect")
                                         .get();

    QVariant variant = r3.value();
    const QDBusArgument v = variant.value<QDBusArgument>();

    int x, y, w, h;
    v.beginStructure();
    v >> x >> y >> w >> h;
    v.endStructure();

    //qDebug() << x << y << w << h;
}

TEST_F(ut_DUtil, testGroups)
{
    auto path = ":/data/dt-settings.json";
    auto settings = DSettings::fromJsonFile(path);

    qDebug() << settings->groupKeys();
    qDebug() << settings->group("shortcuts");
    for (auto cg : settings->group("shortcuts")->childGroups()) {
        qDebug() << cg->key();
    }
    qDebug() << settings->group("shortcuts.ternimal");
    qDebug() << settings->group("shortcuts.ternimal")->options();
}

TEST_F(ut_DUtil, testOsVersion)
{
    DDesktopEntry entry("/tmp/etc/os-version");
    entry.setStringValue("UnionTech OS Desktop", "SystemName", "Version");
    entry.setStringValue("统信桌面操作系统", "SystemName[zh_CN]", "Version");
    entry.setStringValue("Desktop", "ProductType", "Version");
    entry.setStringValue("桌面", "ProductType[zh_CN]", "Version");
    entry.setStringValue("Professional", "EditionName", "Version");
    entry.setStringValue("专业版", "EditionName[zh_CN]", "Version");
    entry.setStringValue("20", "MajorVersion", "Version");
    entry.setStringValue("100A", "MinorVersion", "Version");
    entry.setStringValue("11018.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());

    ASSERT_TRUE(DSysInfo::uosSystemName(QLocale("C")) == "UnionTech OS Desktop");
    ASSERT_TRUE(DSysInfo::uosSystemName(QLocale("zh_CN")) == "统信桌面操作系统");
    ASSERT_TRUE(DSysInfo::uosProductTypeName(QLocale("zh_CN")) == "桌面");
    ASSERT_TRUE(DSysInfo::uosProductTypeName(QLocale("C")) == "Desktop");
    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("zh_CN")) == "专业版");
    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("C")) == "Professional");
    ASSERT_TRUE(DSysInfo::majorVersion() == "20");
    ASSERT_TRUE(DSysInfo::minorVersion() == "100A");
    ASSERT_TRUE(DSysInfo::buildVersion() == "107");

    // test minVersion.BC SP1….SP99
    for (int i = 0; i < 100; ++i) {
        entry.setStringValue(QString("%1").arg(1001 + i * 10), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::spVersion() == (i ? QString("SP%1").arg(i) : QString()));
    }

    // test minVersion.D udpate1~udpate9 updateA~udpateZ
    for (int i = 0; i < 10; ++i) {
        entry.setStringValue(QString("%1").arg(1000 + i), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::udpateVersion() == (i ? QString("update%1").arg(i) : QString()));
    }

    for (char c = 'A'; c <= 'Z'; ++c) {
        entry.setStringValue(QString("100").append(c), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::udpateVersion() == QString("update%1").arg(c));
    }

    // test incalide MinorVersion
    entry.setStringValue(QString("100?"), "MinorVersion", "Version");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::udpateVersion() == QString());
    // restore MinorVersion
    entry.setStringValue(QString("1000"), "MinorVersion", "Version");
    ASSERT_TRUE(entry.save());

    // test OsBuild.B == 1 && OsBuild.D = [1, 6]
    ASSERT_TRUE(DSysInfo::uosType() == DSysInfo::UosDesktop);
    for (int i = 1; i <= 6; ++i) {
        entry.setStringValue(QString("%1").arg(11008.107 + i * 10), "OsBuild", "Version");
        ASSERT_TRUE(entry.save());
        switch (i) {
        case 1:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosProfessional);
            break;
        case 2:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosHome);
            break;
        case 4:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosMilitary);
            break;
        case 5:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosDeviceEdition);
            break;
        case 6:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEducation);
            break;
        default:
            break;
        }
    }

    // test OsBuild.B == 2 && OsBuild.D = [1, 5]
    entry.setStringValue("12018.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::uosType() == DSysInfo::UosServer);
    for (int i = 1; i <= 5; ++i) {
        entry.setStringValue(QString("%1").arg(12008.107 + i * 10), "OsBuild", "Version");
        ASSERT_TRUE(entry.save());
        switch (i) {
        case 1:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEnterprise);
            break;
        case 2:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEnterpriseC);
            break;
        case 3:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEuler);
            break;
        case 4:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosMilitaryS);
            break;
        case 5:
            ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosDeviceEdition);
            break;
        default:
            break;
        }
    }

    // test OsBuild.B == 3
    entry.setStringValue("13018.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::uosType() == DSysInfo::UosDevice);
    ASSERT_TRUE(DSysInfo::uosEditionType() == DSysInfo::UosEnterprise);

    // test invalid OsBuild.B
    entry.setStringValue("10018.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());
    ASSERT_TRUE(DSysInfo::uosType() == DSysInfo::UosTypeUnknown);

    // test OsBuild.E
    for (int i = 0; i < 4; ++i) {
        entry.setStringValue(QString("%1").arg(11000.107 + qreal(1 << i)), "OsBuild", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::uosArch() == (1 << i));
    }

    // 社区版测试
    entry.setStringValue("Community", "EditionName", "Version");
    entry.setStringValue("社区版", "EditionName[zh_CN]", "Version");
    entry.setStringValue("21.1.2", "MinorVersion", "Version");
    entry.setStringValue("11038.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());

    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("zh_CN")) == "社区版");
    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("C")) == "Community");
    ASSERT_TRUE(DSysInfo::minorVersion() == "21.1.2");
    ASSERT_TRUE(DSysInfo::buildVersion() == "107");

    //社区版A_BC_D模式 test minVersion.BC SP1….SP99
    for (int i = 0; i < 100; ++i) {
        entry.setStringValue(QString("%1").arg(1001 + i * 10), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::spVersion() == (i ? QString("SP%1").arg(i) : QString()));
    }

    //社区版A_BC_D模式 test minVersion.D udpate1~udpate9 updateA~udpateZ
    for (int i = 0; i < 10; ++i) {
        entry.setStringValue(QString("%1").arg(1000 + i), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::udpateVersion() == (i ? QString("update%1").arg(i) : QString()));
    }

    //社区版A_B_C模式 test minVersion.BC SP1….SP99
    const QString &defalutSP("21.%1");
    for (int i = 1; i < 100; ++i) {
        entry.setStringValue(defalutSP.arg(i), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::spVersion() == QString("SP%1").arg(i));
    }

    //社区版A_B_C模式 test minVersion.D udpate1~udpate9 updateA~udpateZ
    const QString &defalutUpdate("21.1.%1");
    for (int i = 1; i < 100; ++i) {
        entry.setStringValue(defalutUpdate.arg(i), "MinorVersion", "Version");
        ASSERT_TRUE(entry.save());
        ASSERT_TRUE(DSysInfo::udpateVersion() == QString("update%1").arg(i));
    }

    // 家庭版测试
    entry.setStringValue("Home", "EditionName", "Version");
    entry.setStringValue("家庭版", "EditionName[zh_CN]", "Version");
    entry.setStringValue("21.0", "MinorVersion", "Version");
    entry.setStringValue("11078.107", "OsBuild", "Version");
    ASSERT_TRUE(entry.save());

    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("zh_CN")) == "家庭版");
    ASSERT_TRUE(DSysInfo::uosEditionName(QLocale("C")) == "Home");
    ASSERT_TRUE(DSysInfo::minorVersion() == "21.0");
    ASSERT_TRUE(DSysInfo::buildVersion() == "107");
    ASSERT_TRUE(DSysInfo::spVersion() == QStringLiteral(""));
    ASSERT_TRUE(DSysInfo::udpateVersion() == QStringLiteral(""));

    QFile::remove("/tmp/etc/os-version");
}
