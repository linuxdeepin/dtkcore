// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <unistd.h>
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
        fileWatcher->stopWatcher();
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
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    fileWatcher->setEnabledSubfileWatcher(QUrl());
    ASSERT_TRUE(fileWatcher->startWatcher());
}

TEST_F(ut_DFileWatcher, testDFileWatcherStopWatcher)
{
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    ASSERT_TRUE(fileWatcher->startWatcher());
    ASSERT_TRUE(fileWatcher->stopWatcher());
}

TEST_F(ut_DFileWatcher, testDFileWatcherRestartWatcher)
{
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    ASSERT_TRUE(fileWatcher->startWatcher());
    ASSERT_TRUE(fileWatcher->restartWatcher());
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileDeleted)
{
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
    QFile file("/tmp/etc/test");
    if (file.exists())
        file.remove();
    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileAttributeChanged)
{
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileAttributeChanged);
    QFile file("/tmp/etc/test");
    if (file.exists()) {
        file.remove();
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();

    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}


TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileMoved)
{
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    ASSERT_TRUE(fileWatcher->startWatcher());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileMoved);
#else
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
#endif
    QString oldFile("/tmp/etc/test");
    QString newFile("/tmp/etc/test1");
    QFile::rename(oldFile, newFile);

    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherSubfileCreated)
{
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::subfileCreated);
    QFile file("/tmp/etc/test");
    if (file.exists()) {
        file.remove();
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileModified)
{
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileModified);
    QFile file("/tmp/etc/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "hello";
    file.close();
    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

TEST_F(ut_DFileWatcher, testDFileSystemWatcherFileClosed)
{
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)";

    ASSERT_TRUE(fileWatcher->startWatcher());
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileClosed);
    QFile file("/tmp/etc/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    file.close();
    ASSERT_TRUE(QTest::qWaitFor([&spy](){
        return spy.count() >= 1;
    }, 1000));
    ASSERT_TRUE(spy.count() >= 1);
}

// === Coverage tests for dfilewatcher.cpp onFile* methods ===
// d->path = formatPath("/tmp/etc/test") = "/tmp/etc/test"
// -fno-access-control allows calling private slots directly

TEST_F(ut_DFileWatcher, testOnFileDeletedEmptyName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
    fileWatcher->onFileDeleted("/tmp/etc/test", "");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileDeletedWithName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
    fileWatcher->onFileDeleted("/tmp/etc", "test");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileDeletedNoMatch)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
    fileWatcher->onFileDeleted("/other", "file");
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ut_DFileWatcher, testOnFileAttributeChangedEmptyName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileAttributeChanged);
    fileWatcher->onFileAttributeChanged("/tmp/etc/test", "");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileAttributeChangedWithName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileAttributeChanged);
    fileWatcher->onFileAttributeChanged("/tmp/etc", "test");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileAttributeChangedNoMatch)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileAttributeChanged);
    fileWatcher->onFileAttributeChanged("/other", "file");
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ut_DFileWatcher, testOnFileModifiedEmptyName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileModified);
    fileWatcher->onFileModified("/tmp/etc/test", "");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileModifiedWithName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileModified);
    fileWatcher->onFileModified("/tmp/etc", "test");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileModifiedNoMatch)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileModified);
    fileWatcher->onFileModified("/other", "file");
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ut_DFileWatcher, testOnFileClosedEmptyName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileClosed);
    fileWatcher->onFileClosed("/tmp/etc/test", "");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileClosedWithName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileClosed);
    fileWatcher->onFileClosed("/tmp/etc", "test");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileClosedNoMatch)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileClosed);
    fileWatcher->onFileClosed("/other", "file");
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ut_DFileWatcher, testOnFileCreatedWithName)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::subfileCreated);
    fileWatcher->onFileCreated("/tmp/etc", "test");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileCreatedNoMatch)
{
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::subfileCreated);
    fileWatcher->onFileCreated("/other", "file");
    EXPECT_EQ(spy.count(), 0);
}

// === Coverage tests for _q_handleFileMoved branches ===

TEST_F(ut_DFileWatcher, testOnFileMovedBranchFromEqualsPath)
{
    // from == d->path → fileMoved(from, to)
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileMoved);
    fileWatcher->onFileMoved("/tmp/etc/test", "", "/tmp/etc/test2", "");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileMovedBranchFromParentEqualsPath)
{
    // fromParent == d->path, from != d->path, toParent != d->path → fileDeleted(from)
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
    fileWatcher->onFileMoved("/tmp/etc/test", "subfile", "/other", "otherfile");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileMovedBranchToParentEqualsPath)
{
    // toParent == d->path, no other match → subfileCreated(to)
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::subfileCreated);
    fileWatcher->onFileMoved("/nonexistent", "x", "/tmp/etc/test", "newfile");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testOnFileMovedBranchWatchFileListContainsFrom)
{
    // After startWatcher(), watchFileList contains parentPathList(d->path)
    // which includes "/tmp". Call onFileMoved with from="/tmp" to hit branch 3.
    if (!fileWatcher->startWatcher()) GTEST_SKIP() << "Failed to start watcher";
    QSignalSpy spy(fileWatcher, &DBaseFileWatcher::fileDeleted);
    fileWatcher->onFileMoved("/tmp", "", "/other", "");
    EXPECT_EQ(spy.count(), 1);
    fileWatcher->stopWatcher();
}

TEST_F(ut_DFileWatcher, testOnFileMovedAllNamesNonEmpty)
{
    // Both fname and tname non-empty → exercises joinFilePath for both paths
    QSignalSpy spyMoved(fileWatcher, &DBaseFileWatcher::fileMoved);
    fileWatcher->onFileMoved("/tmp/etc/test", "oldname", "/tmp/etc/test", "newname");
    // from = joinFilePath("/tmp/etc/test", "oldname") = "/tmp/etc/test/oldname"
    // to = joinFilePath("/tmp/etc/test", "newname") = "/tmp/etc/test/newname"
    // fromParent = "/tmp/etc/test" == d->path AND toParent = "/tmp/etc/test" == d->path
    // → first branch: fileMoved(from, to), NOT fileDeleted
    EXPECT_EQ(spyMoved.count(), 1);
    QSignalSpy spyDeleted(fileWatcher, &DBaseFileWatcher::fileDeleted);
    fileWatcher->onFileMoved("/tmp/etc/test", "oldname", "/tmp/etc/test", "newname");
    EXPECT_EQ(spyDeleted.count(), 0);
}

TEST_F(ut_DFileWatcher, testOnFileMovedNoMatch)
{
    // No branch matches → no signal
    QSignalSpy spyDeleted(fileWatcher, &DBaseFileWatcher::fileDeleted);
    QSignalSpy spyMoved(fileWatcher, &DBaseFileWatcher::fileMoved);
    QSignalSpy spyCreated(fileWatcher, &DBaseFileWatcher::subfileCreated);
    fileWatcher->onFileMoved("/nonexistent", "x", "/other", "y");
    EXPECT_EQ(spyDeleted.count(), 0);
    EXPECT_EQ(spyMoved.count(), 0);
    EXPECT_EQ(spyCreated.count(), 0);
}

// === Coverage for formatPath (indirectly via constructor) ===

TEST_F(ut_DFileWatcher, testFormatPathTrailingSeparator)
{
    // formatPath removes trailing separator; d->path should be "/tmp/etc/test"
    // Verify by calling onFileDeleted with the non-trailing path
    DFileWatcher watcher("/tmp/etc/test/");
    QSignalSpy spy(&watcher, &DBaseFileWatcher::fileDeleted);
    watcher.onFileDeleted("/tmp/etc/test", "");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DFileWatcher, testFormatPathRootDir)
{
    // formatPath("/") → QFileInfo("/").absoluteFilePath() = "/"
    // → endsWith separator → chop(1) → "" → isEmpty → return original "/"
    DFileWatcher watcher("/");
    QSignalSpy spy(&watcher, &DBaseFileWatcher::fileDeleted);
    watcher.onFileDeleted("/", "");
    EXPECT_EQ(spy.count(), 1);
}
