// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#include <QProcessEnvironment>
#include "filesystem/dstandardpaths.h"

DCORE_USE_NAMESPACE


class ut_DStandardPaths : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void ut_DStandardPaths::SetUp()
{
    QDir dir("/tmp/etc/");
    if (!dir.exists())
        dir.mkdir("/tmp/etc/");
    DStandardPaths::setMode(DStandardPaths::Snap);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.value("SNAP_USER_COMMON").isEmpty())
        qputenv("SNAP_USER_COMMON", "/tmp/etc");
}

void ut_DStandardPaths::TearDown()
{
    QDir dir("/tmp/etc/");
    if (dir.exists())
        dir.remove("/tmp/etc/");
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.value("SNAP_USER_COMMON") == "/tmp/etc")
        qputenv("SNAP_USER_COMMON", "");
}

TEST_F(ut_DStandardPaths, testDStandardPathsWritableLocation)
{
    QString dir = DStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    ASSERT_TRUE(dir == "/tmp/etc");
}

TEST_F(ut_DStandardPaths, testDStandardPathsStandardLocations)
{
    QStringList dirs = DStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    ASSERT_TRUE(dirs.contains("/tmp/etc"));
}

TEST_F(ut_DStandardPaths, testDStandardPathsXDGPath)
{
    QString home = DStandardPaths::homePath();
    QString dataHome = DStandardPaths::path(DStandardPaths::XDG::DataHome);
    ASSERT_TRUE( (getenv("XDG_DATA_HOME") == dataHome) || (home + "/.local/share") == dataHome );
    QString configHome = DStandardPaths::path(DStandardPaths::XDG::ConfigHome);
    ASSERT_TRUE( getenv("XDG_CONFIG_HOME") == configHome || (home + "/.config") == configHome );
    QString cacheHome = DStandardPaths::path(DStandardPaths::XDG::CacheHome);
    ASSERT_TRUE( getenv("XDG_CACHE_HOME") == cacheHome || (home + "/.cache" == cacheHome) );
    QString rtTime = DStandardPaths::path(DStandardPaths::XDG::RuntimeTime);
    ASSERT_TRUE( getenv("XDG_RUNTIME_DIR") == rtTime || (QStringLiteral("/run/user/") + QString::number(getuid())) == rtTime );
}

TEST_F(ut_DStandardPaths, testDStandardPathsDSGPath)
{
    QByteArray env1 = qgetenv("DSG_DATA_DIRS");
    if(env1.isEmpty())
        qputenv("DSG_DATA_DIRS", "/tmp/DSGDATADIRS");
    QByteArray env2 = qgetenv("DSG_APP_DATA");
    if(env2.isEmpty())
        qputenv("DSG_APP_DATA", "/tmp/DSGAPPDATA");
    QString datadir = DStandardPaths::path(DStandardPaths::DSG::DataDir);
    QString appdata = DStandardPaths::path(DStandardPaths::DSG::AppData);
    ASSERT_TRUE( (getenv("DSG_DATA_DIRS") == datadir) || "/tmp/DSGDATADIRS" == appdata );
    ASSERT_TRUE( getenv("DSG_APP_DATA") == appdata || "/tmp/DSGAPPDATA" == datadir );
}
