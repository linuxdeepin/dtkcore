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
#include "filesystem/dfilesystemwatcher.h"

DCORE_USE_NAMESPACE


class ut_DFileSystemWatcher : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DFileSystemWatcher *fileSystemWatcher = nullptr;

};

void ut_DFileSystemWatcher::SetUp()
{
    fileSystemWatcher = new DFileSystemWatcher(nullptr);
    QDir dir0("/tmp/etc0/");
    if (!dir0.exists())
        dir0.mkdir("/tmp/etc0/");
    QDir dir1("/tmp/etc1/");
    if (!dir1.exists())
        dir1.mkdir("/tmp/etc1/");
}

void ut_DFileSystemWatcher::TearDown()
{
    if (fileSystemWatcher) {
        delete fileSystemWatcher;
        fileSystemWatcher = nullptr;
    }
    QDir dir0("/tmp/etc0/");
    if (dir0.exists())
        dir0.remove("/tmp/etc0/");
    QDir dir1("/tmp/etc1/");
    if (dir1.exists())
        dir1.remove("/tmp/etc1/");
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherAddPath)
{
    fileSystemWatcher->addPath("/tmp/etc0");
    QStringList dirs = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs.contains("/tmp/etc0"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherAddPaths)
{
    fileSystemWatcher->addPaths( QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs.contains("/tmp/etc0"));
    ASSERT_TRUE(dirs.contains("/tmp/etc1"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherRemovePath)
{
    fileSystemWatcher->addPath("/tmp/etc0");
    QStringList dirs0 = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs0.contains("/tmp/etc0"));
    fileSystemWatcher->removePath("/tmp/etc0");
    QStringList dirs1 = fileSystemWatcher->directories();
    ASSERT_FALSE(dirs1.contains("/tmp/etc0"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherRemovePaths)
{
    fileSystemWatcher->addPaths( QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs0 = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs0.contains("/tmp/etc0"));
    ASSERT_TRUE(dirs0.contains("/tmp/etc1"));
    fileSystemWatcher->removePaths(QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs1 = fileSystemWatcher->directories();
    ASSERT_FALSE(dirs1.contains("/tmp/etc0"));
    ASSERT_FALSE(dirs1.contains("/tmp/etc1"));
}
