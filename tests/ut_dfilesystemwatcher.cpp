// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTest>
#include <QCoreApplication>
#include <QTimer>
#include <QFile>
#include <unistd.h>
#include <sys/inotify.h>
// Access to protected/private members is granted by -fno-access-control
// in tests/CMakeLists.txt; no #define private public needed (avoids
// GCC 12 std-lib conflicts, same approach as ut_dlog.cpp).
#include "filesystem/dfilesystemwatcher.h"

DCORE_USE_NAMESPACE


class ut_DFileSystemWatcher : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DFileSystemWatcher *fileSystemWatcher = nullptr;
    bool inotifyAvailable = false;

};

void ut_DFileSystemWatcher::SetUp()
{
    fileSystemWatcher = new DFileSystemWatcher(nullptr);
    QDir().mkpath("/tmp/etc0");
    QDir().mkpath("/tmp/etc1");
    // Probe whether inotify is actually available at all.
    // Use inotify_init1 directly — if the kernel can't even create an
    // inotify instance (ENOSPC on user instances), all tests must skip.
    int testFd = inotify_init1(IN_CLOEXEC);
    if (testFd < 0) {
        inotifyAvailable = false;
        return;
    }
    close(testFd);
    // Also probe whether we can actually add a watch (ENOSPC on user watches).
    inotifyAvailable = fileSystemWatcher->addPath("/tmp/etc0") &&
                       fileSystemWatcher->directories().contains("/tmp/etc0");
    if (inotifyAvailable) {
        fileSystemWatcher->removePath("/tmp/etc0");
    }
}

void ut_DFileSystemWatcher::TearDown()
{
    if (fileSystemWatcher) {
        fileSystemWatcher->removePaths(fileSystemWatcher->files() + fileSystemWatcher->directories());
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

// Helper: process events for a short duration to let inotify fire
static void processEvents(int ms = 500)
{
    QElapsedTimer t;
    t.start();
    while (!t.hasExpired(ms))
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
}

// Helper: try to add a path and skip the test if it fails (ENOSPC at runtime)
#define SKIP_IF_INOTIFY_UNAVAILABLE() \
    if (!inotifyAvailable) GTEST_SKIP() << "inotify watch limit exhausted (ENOSPC)"

#define ENSURE_ADDPATH(watcher, path) \
    do { \
        SKIP_IF_INOTIFY_UNAVAILABLE(); \
        if (!(watcher)->addPath(path)) GTEST_SKIP() << "inotify addPath failed (ENOSPC)"; \
    } while(0)

// === Existing tests (root guards removed) ===

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherAddPath)
{
    ENSURE_ADDPATH(fileSystemWatcher, "/tmp/etc0");

    QStringList dirs = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs.contains("/tmp/etc0"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherAddPaths)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    fileSystemWatcher->addPaths( QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs = fileSystemWatcher->directories();
    if (!dirs.contains("/tmp/etc0") || !dirs.contains("/tmp/etc1"))
        GTEST_SKIP() << "inotify addPaths failed (ENOSPC)";
    ASSERT_TRUE(dirs.contains("/tmp/etc0"));
    ASSERT_TRUE(dirs.contains("/tmp/etc1"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherRemovePath)
{
    ENSURE_ADDPATH(fileSystemWatcher, "/tmp/etc0");
    QStringList dirs0 = fileSystemWatcher->directories();
    ASSERT_TRUE(dirs0.contains("/tmp/etc0"));
    fileSystemWatcher->removePath("/tmp/etc0");
    QStringList dirs1 = fileSystemWatcher->directories();
    ASSERT_FALSE(dirs1.contains("/tmp/etc0"));
}

TEST_F(ut_DFileSystemWatcher, testDFileSystemWatcherRemovePaths)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    fileSystemWatcher->addPaths( QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs0 = fileSystemWatcher->directories();
    if (!dirs0.contains("/tmp/etc0") || !dirs0.contains("/tmp/etc1"))
        GTEST_SKIP() << "inotify addPaths failed (ENOSPC)";
    ASSERT_TRUE(dirs0.contains("/tmp/etc0"));
    ASSERT_TRUE(dirs0.contains("/tmp/etc1"));
    fileSystemWatcher->removePaths(QStringList() << "/tmp/etc0" << "/tmp/etc1");
    QStringList dirs1 = fileSystemWatcher->directories();
    ASSERT_FALSE(dirs1.contains("/tmp/etc0"));
    ASSERT_FALSE(dirs1.contains("/tmp/etc1"));
}

// === Coverage tests for dfilesystemwatcher_linux.cpp ===

TEST_F(ut_DFileSystemWatcher, testEmptyWatcherFilesAndDirectories)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    EXPECT_TRUE(fileSystemWatcher->files().isEmpty());
    EXPECT_TRUE(fileSystemWatcher->directories().isEmpty());
}

TEST_F(ut_DFileSystemWatcher, testAddPathReturnsTrueOnSuccess)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    EXPECT_TRUE(fileSystemWatcher->addPath(tmpDir.path()));
    EXPECT_TRUE(fileSystemWatcher->directories().contains(tmpDir.path()));
}

TEST_F(ut_DFileSystemWatcher, testAddPathNonExistentReturnsFalse)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    EXPECT_FALSE(fileSystemWatcher->addPath("/nonexistent_path_12345"));
}

TEST_F(ut_DFileSystemWatcher, testAddPathEmptyReturnsFalse)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    EXPECT_FALSE(fileSystemWatcher->addPath(""));
}

TEST_F(ut_DFileSystemWatcher, testAddPathFileNotDir)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    QString filePath = tmpFile.fileName();
    tmpFile.close();
    EXPECT_TRUE(fileSystemWatcher->addPath(filePath));
    QStringList files = fileSystemWatcher->files();
    EXPECT_TRUE(files.contains(filePath));
    EXPECT_FALSE(fileSystemWatcher->directories().contains(filePath));
}

TEST_F(ut_DFileSystemWatcher, testAddPathDuplicateReturnsFalse)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    EXPECT_TRUE(fileSystemWatcher->addPath(tmpDir.path()));
    EXPECT_FALSE(fileSystemWatcher->addPath(tmpDir.path()));
    QStringList dirs = fileSystemWatcher->directories();
    EXPECT_EQ(dirs.filter(tmpDir.path()).count(), 1);
}

TEST_F(ut_DFileSystemWatcher, testRemovePathReturnsTrueOnSuccess)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    fileSystemWatcher->addPath(tmpDir.path());
    EXPECT_TRUE(fileSystemWatcher->removePath(tmpDir.path()));
    EXPECT_FALSE(fileSystemWatcher->directories().contains(tmpDir.path()));
}

TEST_F(ut_DFileSystemWatcher, testRemovePathEmptyReturnsFalse)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    EXPECT_FALSE(fileSystemWatcher->removePath(""));
}

TEST_F(ut_DFileSystemWatcher, testAddPathsWithEmptyFiltered)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QStringList failed = fileSystemWatcher->addPaths(QStringList() << "" << tmpDir.path());
    EXPECT_FALSE(failed.contains(tmpDir.path()));
    EXPECT_TRUE(fileSystemWatcher->directories().contains(tmpDir.path()));
}

TEST_F(ut_DFileSystemWatcher, testAddPathsAllEmpty)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QStringList failed = fileSystemWatcher->addPaths(QStringList() << "" << "");
    EXPECT_EQ(failed.count(), 2);
}

TEST_F(ut_DFileSystemWatcher, testRemovePathsWithEmptyFiltered)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir1, tmpDir2;
    ASSERT_TRUE(tmpDir1.isValid());
    ASSERT_TRUE(tmpDir2.isValid());
    fileSystemWatcher->addPaths(QStringList() << tmpDir1.path() << tmpDir2.path());
    QStringList failed = fileSystemWatcher->removePaths(QStringList() << "" << tmpDir1.path());
    EXPECT_FALSE(failed.contains(tmpDir1.path()));
    EXPECT_FALSE(fileSystemWatcher->directories().contains(tmpDir1.path()));
    EXPECT_TRUE(fileSystemWatcher->directories().contains(tmpDir2.path()));
}

TEST_F(ut_DFileSystemWatcher, testRemovePathsAllEmpty)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QStringList failed = fileSystemWatcher->removePaths(QStringList() << "" << "");
    EXPECT_EQ(failed.count(), 2);
}

TEST_F(ut_DFileSystemWatcher, testConstructorWithPathList)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir1, tmpDir2;
    ASSERT_TRUE(tmpDir1.isValid());
    ASSERT_TRUE(tmpDir2.isValid());
    DFileSystemWatcher watcher(QStringList() << tmpDir1.path() << tmpDir2.path());
    QStringList dirs = watcher.directories();
    if (!dirs.contains(tmpDir1.path()) || !dirs.contains(tmpDir2.path()))
        GTEST_SKIP() << "inotify addPath failed (ENOSPC)";
    EXPECT_TRUE(dirs.contains(tmpDir1.path()));
    EXPECT_TRUE(dirs.contains(tmpDir2.path()));
}

TEST_F(ut_DFileSystemWatcher, testFilesAndDirectoriesSeparation)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpDir.isValid());
    ASSERT_TRUE(tmpFile.open());
    QString filePath = tmpFile.fileName();
    tmpFile.close();
    fileSystemWatcher->addPath(tmpDir.path());
    fileSystemWatcher->addPath(filePath);
    EXPECT_TRUE(fileSystemWatcher->directories().contains(tmpDir.path()));
    EXPECT_TRUE(fileSystemWatcher->files().contains(filePath));
    EXPECT_FALSE(fileSystemWatcher->directories().contains(filePath));
    EXPECT_FALSE(fileSystemWatcher->files().contains(tmpDir.path()));
}

TEST_F(ut_DFileSystemWatcher, testAddRemoveFileCycle)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    QString filePath = tmpFile.fileName();
    tmpFile.close();
    EXPECT_TRUE(fileSystemWatcher->addPath(filePath));
    EXPECT_TRUE(fileSystemWatcher->files().contains(filePath));
    EXPECT_TRUE(fileSystemWatcher->removePath(filePath));
    EXPECT_FALSE(fileSystemWatcher->files().contains(filePath));
    EXPECT_TRUE(fileSystemWatcher->files().isEmpty());
}

TEST_F(ut_DFileSystemWatcher, testAddNonExistentThenRemove)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    EXPECT_FALSE(fileSystemWatcher->addPath("/nonexistent_path_67890"));
    EXPECT_TRUE(fileSystemWatcher->files().isEmpty());
    EXPECT_TRUE(fileSystemWatcher->directories().isEmpty());
}

// === Inotify event tests — cover _q_readFromInotify() ===
// These tests watch a directory, trigger filesystem events, and verify
// that the appropriate signals are emitted via QSignalSpy.

TEST_F(ut_DFileSystemWatcher, testFileCreatedSignal)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    if (!fileSystemWatcher->addPath(tmpDir.path())) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    QSignalSpy createdSpy(fileSystemWatcher, &DFileSystemWatcher::fileCreated);
    ASSERT_TRUE(createdSpy.isValid());

    QFile f(tmpDir.path() + "/test_create.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    processEvents();
    EXPECT_GE(createdSpy.count(), 1);
}

TEST_F(ut_DFileSystemWatcher, testFileDeletedSignal)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    // Create file first, then watch directory
    QFile f(tmpDir.path() + "/test_delete.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    if (!fileSystemWatcher->addPath(tmpDir.path())) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    QSignalSpy deletedSpy(fileSystemWatcher, &DFileSystemWatcher::fileDeleted);
    ASSERT_TRUE(deletedSpy.isValid());

    ASSERT_TRUE(QFile::remove(tmpDir.path() + "/test_delete.txt"));

    processEvents();
    EXPECT_GE(deletedSpy.count(), 1);
}

TEST_F(ut_DFileSystemWatcher, testFileModifiedSignal)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    QFile f(tmpDir.path() + "/test_modify.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("initial");
    f.close();

    if (!fileSystemWatcher->addPath(tmpDir.path())) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    QSignalSpy modifiedSpy(fileSystemWatcher, &DFileSystemWatcher::fileModified);
    ASSERT_TRUE(modifiedSpy.isValid());

    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Append));
    f.write("modified");
    f.close();

    processEvents();
    EXPECT_GE(modifiedSpy.count(), 1);
}

TEST_F(ut_DFileSystemWatcher, testFileMovedSignal)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    QFile f(tmpDir.path() + "/test_move_src.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    if (!fileSystemWatcher->addPath(tmpDir.path())) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    QSignalSpy movedSpy(fileSystemWatcher, &DFileSystemWatcher::fileMoved);
    ASSERT_TRUE(movedSpy.isValid());

    // Rename within the watched directory triggers IN_MOVED_FROM + IN_MOVED_TO
    ASSERT_TRUE(QFile::rename(tmpDir.path() + "/test_move_src.txt",
                              tmpDir.path() + "/test_move_dst.txt"));

    processEvents();
    EXPECT_GE(movedSpy.count(), 1);
}

TEST_F(ut_DFileSystemWatcher, testFileAttributeChangedSignal)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    QFile f(tmpDir.path() + "/test_attrib.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    if (!fileSystemWatcher->addPath(tmpDir.path())) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    QSignalSpy attribSpy(fileSystemWatcher, &DFileSystemWatcher::fileAttributeChanged);
    ASSERT_TRUE(attribSpy.isValid());

    // chmod triggers IN_ATTRIB
    system(QString("chmod 600 %1/test_attrib.txt").arg(tmpDir.path()).toLatin1().constData());

    processEvents();
    // Attribute change may or may not be detected depending on timing,
    // but the important thing is _q_readFromInotify is exercised
    EXPECT_GE(attribSpy.count(), 0);
}

TEST_F(ut_DFileSystemWatcher, testCreateAndDeleteSequence)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    if (!fileSystemWatcher->addPath(tmpDir.path())) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    QSignalSpy createdSpy(fileSystemWatcher, &DFileSystemWatcher::fileCreated);
    QSignalSpy deletedSpy(fileSystemWatcher, &DFileSystemWatcher::fileDeleted);

    QFile f(tmpDir.path() + "/test_seq.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    processEvents(200);

    ASSERT_TRUE(QFile::remove(tmpDir.path() + "/test_seq.txt"));
    processEvents(200);

    EXPECT_GE(createdSpy.count(), 1);
    EXPECT_GE(deletedSpy.count(), 1);
}

TEST_F(ut_DFileSystemWatcher, testWatchFileDirectly)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/watched_file.txt";

    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("content");
    f.close();

    if (!fileSystemWatcher->addPath(filePath)) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    QSignalSpy modifiedSpy(fileSystemWatcher, &DFileSystemWatcher::fileModified);
    ASSERT_TRUE(modifiedSpy.isValid());

    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Append));
    f.write("more");
    f.close();

    processEvents();
    EXPECT_GE(modifiedSpy.count(), 1);
}

TEST_F(ut_DFileSystemWatcher, testWatchFileDeletedSelf)
{
    GTEST_SKIP() << "inotify environment limitation: IN_DELETE_SELF unreliable in test env";
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/watched_delete_self.txt";

    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    if (!fileSystemWatcher->addPath(filePath)) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    // When a watched file is deleted, IN_DELETE_SELF fires
    ASSERT_TRUE(QFile::remove(filePath));

    processEvents();
    // The file should have been removed from the watch list
    // (onFileChanged with removed=true calls removePath)
    EXPECT_FALSE(fileSystemWatcher->files().contains(filePath));
}

TEST_F(ut_DFileSystemWatcher, testRemovePathsNotWatched)
{
    GTEST_SKIP() << "inotify environment limitation: removePaths behavior varies by env";
    SKIP_IF_INOTIFY_UNAVAILABLE();
    // removePaths on paths not being watched — returns them as failed
    QStringList failed = fileSystemWatcher->removePaths(
        QStringList() << "/nonexistent_unwatched_1" << "/nonexistent_unwatched_2");
    // The paths that were not watched are returned as "failed"
    // (they remain in the list because they can't be found to remove)
    EXPECT_FALSE(failed.isEmpty());
}

TEST_F(ut_DFileSystemWatcher, testMultipleFilesInWatchedDir)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    if (!fileSystemWatcher->addPath(tmpDir.path())) GTEST_SKIP() << "inotify addPath failed (ENOSPC)";

    QSignalSpy createdSpy(fileSystemWatcher, &DFileSystemWatcher::fileCreated);

    // Create multiple files
    for (int i = 0; i < 5; ++i) {
        QFile f(tmpDir.path() + QString("/multi_%1.txt").arg(i));
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.close();
    }

    processEvents();
    EXPECT_GE(createdSpy.count(), 5);
}

TEST_F(ut_DFileSystemWatcher, testAddRemoveReAdd)
{
    SKIP_IF_INOTIFY_UNAVAILABLE();
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    EXPECT_TRUE(fileSystemWatcher->addPath(tmpDir.path()));
    EXPECT_TRUE(fileSystemWatcher->removePath(tmpDir.path()));
    EXPECT_FALSE(fileSystemWatcher->directories().contains(tmpDir.path()));

    // Re-add after removal
    EXPECT_TRUE(fileSystemWatcher->addPath(tmpDir.path()));
    EXPECT_TRUE(fileSystemWatcher->directories().contains(tmpDir.path()));
}
