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
