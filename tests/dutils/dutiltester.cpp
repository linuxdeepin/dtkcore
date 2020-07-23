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

#include "dutiltester.h"

#include <QCoreApplication>
#include <QtTest/QtTest>
#include <QStandardPaths>
#include <QThread>
#include <QDBusPendingCall>
#include <QDBusReply>

#include "log/LogManager.h"
#include "filesystem/dpathbuf.h"
#include "singletontester.h"
#include "util/dtimeunitformatter.h"
#include "util/ddisksizeformatter.h"
#include "util/ddbussender.h"
#include "settings/dsettings.h"
#include "settings/dsettingsgroup.h"
#include "settings/dsettingsoption.h"
#include "dsysinfo.h"

DCORE_USE_NAMESPACE

void TestDUtil::testLogPath()
{
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName("deepin-test-dtk");

    DPathBuf logPath(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());

#ifdef Q_OS_OSX
    logPath = logPath / "Library" / "Caches" / "deepin" / "deepin-test-dtk" / "deepin-test-dtk.log";
#else
    logPath = logPath / ".cache" / "deepin" / "deepin-test-dtk" / "deepin-test-dtk.log";
#endif

    QCOMPARE(DLogManager::getlogFilePath(), logPath.toString());
}

void TestDUtil::testPathChange()
{
    DPathBuf root("/");

    auto usr = root / "./usr";
    QCOMPARE(QDir(usr.toString()).absolutePath(), QDir::toNativeSeparators("/usr"));

    root /= "root";
    QCOMPARE(root.toString(), QDir::toNativeSeparators("/root"));

    root /= "../usr";
    QCOMPARE(root.toString(), usr.toString());
}

void TestDUtil::testDSingleton()
{
    auto threadA = new QThread;
    auto testerA = new MultiSingletonTester;
    connect(threadA, &QThread::started, testerA, &MultiSingletonTester::run);
    testerA->moveToThread(threadA);

    auto threadB = new QThread;
    auto testerB = new MultiSingletonTester;
    testerB->moveToThread(threadB);
    connect(threadB, &QThread::started, testerB, &MultiSingletonTester::run);

    threadA->start();
    threadB->start();

    QThread::sleep(5);
}

void TestDUtil::testTimeFormatter()
{
    const DTimeUnitFormatter timeFormatter;

    // 3600 seconds == 1 hour
    const auto r0 = timeFormatter.format(3600, DTimeUnitFormatter::Seconds);
    Q_ASSERT(r0.first == 1 && r0.second == DTimeUnitFormatter::Hour);

    // 86400 seconds == 1 day
    const auto r1 = timeFormatter.format(86400, DTimeUnitFormatter::Seconds);
    Q_ASSERT(r1.first == 1 && r1.second == DTimeUnitFormatter::Day);

    // 129600 seconds == 1.5 day
    const auto r3 = timeFormatter.format(129600, DTimeUnitFormatter::Seconds);
    Q_ASSERT(qFuzzyCompare(1.5, r3.first) && r3.second == DTimeUnitFormatter::Day);

    // 1.5 day == 36 hours
    const auto r4 = timeFormatter.formatAs(1.5, DTimeUnitFormatter::Day, DTimeUnitFormatter::Hour);
    Q_ASSERT(r4 == 36);
}

void TestDUtil::testTimeFormatterList()
{
    const DTimeUnitFormatter timeFormatter;

    // 135120.5 Minutes == 93 days + 20 hours + 30 seconds
    const auto r = timeFormatter.formatAsUnitList(135120.5, DTimeUnitFormatter::Minute);
    Q_ASSERT(r[0].first == 93 && r[0].second == DTimeUnitFormatter::Day);
    Q_ASSERT(r[1].first == 20 && r[1].second == DTimeUnitFormatter::Hour);
    Q_ASSERT(r[2].first == 30 && r[2].second == DTimeUnitFormatter::Seconds);
}

void TestDUtil::testDiskFormatter()
{
    const DDiskSizeFormatter diskFormatter1000 = DDiskSizeFormatter();

    // 1000 K == 1 M
    const auto i0 = diskFormatter1000.format(1000, DDiskSizeFormatter::K);
    Q_ASSERT(i0.first == 1 && i0.second == DDiskSizeFormatter::M);

    // 1000 K == 1000000 B
    const auto i1 = diskFormatter1000.formatAs(1000, DDiskSizeFormatter::K, DDiskSizeFormatter::B);
    Q_ASSERT(i1 == 1000000);
}

void TestDUtil::testDiskFormatterList()
{
    const DDiskSizeFormatter diskFormatter = DDiskSizeFormatter();

    // 1351223412.1234 KB == 1 TB + 351 GB + 223 MB + 412 KB + 123.4 B
    const auto r = diskFormatter.formatAsUnitList(1351223412.1234, DDiskSizeFormatter::K);
    Q_ASSERT(r[0].first == 1 && r[0].second == DDiskSizeFormatter::T);
    Q_ASSERT(r[1].first == 351 && r[1].second == DDiskSizeFormatter::G);
    Q_ASSERT(r[2].first == 223 && r[2].second == DDiskSizeFormatter::M);
    Q_ASSERT(r[3].first == 412 && r[3].second == DDiskSizeFormatter::K);

    // TODO: test failed
//    Q_ASSERT(r[4].first == 123.4 && r[4].second == DiskSizeFormatter::B);
}

void TestDUtil::testDiskFormatter1024()
{
    const DDiskSizeFormatter diskFormatter = DDiskSizeFormatter().rate(1024);

    // 1024 K == 1 M
    const auto d0 = diskFormatter.format(1024, DDiskSizeFormatter::K);
    Q_ASSERT(d0.first == 1 && d0.second == DDiskSizeFormatter::M);

    // 100000000000 B == 93.13225746154785 G
    const auto d1 = diskFormatter.format(100000000000, DDiskSizeFormatter::B);
    Q_ASSERT(qFuzzyCompare(93.13225746154785, d1.first) && d1.second == DDiskSizeFormatter::G);

    // 100000000000 B == 0.09094947017729282 T
    const auto d2 = diskFormatter.formatAs(100000000000, DDiskSizeFormatter::B, DDiskSizeFormatter::T);
    Q_ASSERT(qFuzzyCompare(0.09094947017729282, d2));
}

void TestDUtil::testDBusSender()
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
        Q_ASSERT(r2.value().toInt() == 1);
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

    qDebug() << x << y << w << h;
}

void TestDUtil::testGroups()
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

void TestDUtil::testSysInfo()
{
    qDebug() << DSysInfo::uosType() <<
                DSysInfo::uosEditionType() <<
                DSysInfo::uosArch() <<
                DSysInfo::uosProductTypeName() <<
                DSysInfo::uosSystemName() <<
                DSysInfo::uosEditionName() <<
                DSysInfo::spVersion() <<
                DSysInfo::udpateVersion() <<
                DSysInfo::majorVersion() <<
                DSysInfo::minorVersion() <<
                DSysInfo::buildVersion() ;
}
