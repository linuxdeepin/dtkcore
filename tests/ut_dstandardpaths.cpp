/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Wang Fei <wangfeia@uniontech.com>
 *
 * Maintainer: Wang Fei <wangfeia@uniontech.com>
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
