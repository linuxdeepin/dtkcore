// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QSignalSpy>
#include <QTest>
#include "filesystem/dfilewatcher.h"
#include "filesystem/dfilewatchermanager.h"

DCORE_USE_NAMESPACE

class ut_DFileWatcherManager : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DFileWatcherManager *fileWatcherManager = nullptr;
};

void ut_DFileWatcherManager::SetUp()
{
    fileWatcherManager = new DFileWatcherManager(nullptr);
    QFile file("/tmp/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();
}

void ut_DFileWatcherManager::TearDown()
{
    if (fileWatcherManager) {
        delete fileWatcherManager;
        fileWatcherManager = nullptr;
    }
    QFile file("/tmp/test");
    if (file.exists())
        file.remove();
    QFile file1("/tmp/test1");
    if (file1.exists())
        file1.remove();
}

TEST_F(ut_DFileWatcherManager, testDFileWatcherManagerAdd)
{
    auto watcher = fileWatcherManager->add("/tmp/test");
    if (!watcher->startWatcher())
        return;

    // test fileDeleted signal
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileDeleted);
    QFile file("/tmp/test");
    if (file.exists())
        file.remove();
    ASSERT_TRUE(QTest::qWaitFor([&spy]() { return spy.count() >= 1; }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcherManager, testDFileWatcherManagerRemove)
{
    fileWatcherManager->add("/tmp/test");
    fileWatcherManager->remove("/tmp/test");
    ASSERT_EQ(fileWatcherManager->watchedFiles().count(), 0);
}

TEST_F(ut_DFileWatcherManager, testDFileWatcherManagerRemoveAll)
{
    fileWatcherManager->add("/tmp/test");
    fileWatcherManager->add("/tmp/test1");
    fileWatcherManager->removeAll();
    ASSERT_EQ(fileWatcherManager->watchedFiles().count(), 0);
}

TEST_F(ut_DFileWatcherManager, testDFileSystemWatcherwatchedFiles)
{
    fileWatcherManager->add("/tmp/test");
    fileWatcherManager->add("/tmp/test1");
    fileWatcherManager->watchedFiles();
    ASSERT_EQ(fileWatcherManager->watchedFiles().count(), 2);
}
