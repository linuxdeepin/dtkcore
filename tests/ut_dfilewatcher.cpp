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
#include <QSignalSpy>
#include <QTest>
#include <QUrl>
#include "filesystem/dfilewatcher.h"

DCORE_USE_NAMESPACE


class ut_DFileWatcher : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DFileWatcher *fileWatcher = nullptr;

};

void ut_DFileWatcher::SetUp()
{
    QDir dir("/tmp/etc/");
    if (!dir.exists())
        dir.mkdir("/tmp/etc/");
    QFile file("/tmp/etc/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();
    fileWatcher = new DFileWatcher("/tmp/etc/test");

}

void ut_DFileWatcher::TearDown()
{
    if (fileWatcher) {
        delete fileWatcher;
        fileWatcher = nullptr;
    }
    QDir dir("/tmp/etc/");
    if (dir.exists())
        dir.remove("/tmp/etc/");
    QFile file("/tmp/etc/test");
    if (file.exists())
        file.remove();
    QFile file1("/tmp/etc/test1");
    if (file1.exists())
        file1.remove();
}

TEST_F(ut_DFileWatcher, testDFileWatcherFileUrl)
{
    QUrl url = fileWatcher->fileUrl();
    ASSERT_TRUE(url.toString() == "file:///tmp/etc/test");
}

TEST_F(ut_DFileWatcher, testDFileWatcherStartWatcher)
{
    fileWatcher->setEnabledSubfileWatcher(QUrl());
    ASSERT_TRUE(fileWatcher->startWatcher());
}

TEST_F(ut_DFileWatcher, testDFileWatcherStopWatcher)
{
    ASSERT_TRUE(fileWatcher->startWatcher());
    ASSERT_TRUE(fileWatcher->stopWatcher());
}

TEST_F(ut_DFileWatcher, testDFileWatcherRestartWatcher)
{
    ASSERT_TRUE(fileWatcher->startWatcher());
    ASSERT_TRUE(fileWatcher->restartWatcher());
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileDeleted)
{
    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
    QFile file("/tmp/etc/test");
    if (file.exists())
        file.remove();
    QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000);
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileAttributeChanged)
{
    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileAttributeChanged);
    QFile file("/tmp/etc/test");
    if (file.exists()) {
        file.remove();
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();

    QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000);
    ASSERT_TRUE(spy.count() >= 1);
}


TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileMoved)
{
    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileMoved);
    QString oldFile("/tmp/etc/test");
    QString newFile("/tmp/etc/test1");
    QFile::rename(oldFile, newFile);

    QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000);
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherSubfileCreated)
{
    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::subfileCreated);
    QFile file("/tmp/etc/test");
    if (file.exists()) {
        file.remove();
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000);
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileModified)
{
    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileModified);
    QFile file("/tmp/etc/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "hello";
    file.close();
    QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000);
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileClosed)
{
    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileClosed);
    QFile file("/tmp/etc/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    file.close();
    QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000);
    ASSERT_TRUE(spy.count() >= 1);
}
