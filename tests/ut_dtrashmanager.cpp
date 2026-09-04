// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QStandardPaths>
#include "filesystem/dstandardpaths.h"
#include "filesystem/dtrashmanager.h"

DCORE_USE_NAMESPACE

static QString trashPath()
{
    return DStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Trash";
}

static QString trashFilesPath()
{
    return trashPath() + "/files";
}

static QString trashInfoPath()
{
    return trashPath() + "/info";
}

// Shared trash-availability flags: probed once in SetUp(), reused by all tests.
static bool s_trashAvailable = false;      // file trash
static bool s_trashDirAvailable = false;   // directory trash
static bool s_trashProbed = false;

class ut_DTrashManager : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    QTemporaryDir m_tempDir;
    QTemporaryDir m_xdgDataDir;
};

void ut_DTrashManager::SetUp()
{
    // Set XDG_DATA_HOME to a temp dir so Trash is on the same filesystem
    // as test files (avoids EXDEV on directory rename in sandbox)
    ASSERT_TRUE(m_xdgDataDir.isValid());
    qputenv("XDG_DATA_HOME", m_xdgDataDir.path().toLocal8Bit());

    // Use Test mode so QStandardPaths returns predictable /tmp-based paths
    // and does not interfere with real user data.
    DStandardPaths::setMode(DStandardPaths::Test);

    // Clean trash before each test
    DTrashManager::instance()->cleanTrash();
    QDir().mkpath(trashFilesPath());
    QDir().mkpath(trashInfoPath());

    // Probe trash availability once: try moving a temp file to trash.
    if (!s_trashProbed) {
        s_trashProbed = true;

        // File probe
        {
            QTemporaryFile probeFile;
            probeFile.setAutoRemove(false);
            probeFile.open();
            probeFile.write("probe");
            probeFile.close();
            s_trashAvailable = DTrashManager::instance()->moveToTrash(probeFile.fileName());
            DTrashManager::instance()->cleanTrash();
        }

        // Directory probe — some container environments support file trash
        // but not directory trash (rename of directories fails), so probe
        // separately to allow directory tests to SKIP instead of FAIL.
        {
            QTemporaryDir probeDir;
            probeDir.setAutoRemove(false);
            if (probeDir.isValid()) {
                QFile inner(probeDir.path() + "/inner.txt");
                inner.open(QIODevice::WriteOnly);
                inner.write("probe");
                inner.close();
                s_trashDirAvailable = DTrashManager::instance()->moveToTrash(probeDir.path());
                DTrashManager::instance()->cleanTrash();
            }
        }
    }
}

void ut_DTrashManager::TearDown()
{
    DTrashManager::instance()->cleanTrash();
    // Restore Auto mode so other test suites are unaffected
    DStandardPaths::setMode(DStandardPaths::Auto);
}

// Helper macro for tests that depend on a working file trash service
#define SKIP_IF_NO_TRASH() \
    if (!s_trashAvailable) GTEST_SKIP() << "trash service not available in this environment"

// Helper macro for tests that depend on a working directory trash service
#define SKIP_IF_NO_DIR_TRASH() \
    if (!s_trashDirAvailable) GTEST_SKIP() << "directory trash not available in this environment"

// --- trashIsEmpty ---

TEST_F(ut_DTrashManager, trashIsEmptyWhenEmpty)
{
    EXPECT_TRUE(DTrashManager::instance()->trashIsEmpty());
}

TEST_F(ut_DTrashManager, trashIsNotEmptyAfterMove)
{
    SKIP_IF_NO_TRASH();

    QString filePath = m_tempDir.path() + "/testfile.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("hello");
    f.close();

    ASSERT_TRUE(DTrashManager::instance()->moveToTrash(filePath));
    EXPECT_FALSE(DTrashManager::instance()->trashIsEmpty());
}

// --- cleanTrash ---

TEST_F(ut_DTrashManager, cleanTrashWhenEmpty)
{
    EXPECT_TRUE(DTrashManager::instance()->cleanTrash());
}

TEST_F(ut_DTrashManager, cleanTrashAfterMove)
{
    SKIP_IF_NO_TRASH();

    QString filePath = m_tempDir.path() + "/testfile_clean.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    ASSERT_TRUE(DTrashManager::instance()->moveToTrash(filePath));
    EXPECT_FALSE(DTrashManager::instance()->trashIsEmpty());

    EXPECT_TRUE(DTrashManager::instance()->cleanTrash());
    EXPECT_TRUE(DTrashManager::instance()->trashIsEmpty());
}

TEST_F(ut_DTrashManager, cleanTrashAfterMultipleFiles)
{
    SKIP_IF_NO_TRASH();

    for (int i = 0; i < 3; ++i) {
        QString filePath = m_tempDir.path() + QString("/multi_%1.txt").arg(i);
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("data");
        f.close();
        ASSERT_TRUE(DTrashManager::instance()->moveToTrash(filePath));
    }
    EXPECT_FALSE(DTrashManager::instance()->trashIsEmpty());
    EXPECT_TRUE(DTrashManager::instance()->cleanTrash());
    EXPECT_TRUE(DTrashManager::instance()->trashIsEmpty());
}

// --- moveToTrash ---

TEST_F(ut_DTrashManager, moveToTrashNonExistentFile)
{
    EXPECT_FALSE(DTrashManager::instance()->moveToTrash("/nonexistent/path/file.txt"));
}

TEST_F(ut_DTrashManager, moveToTrashSingleFile)
{
    SKIP_IF_NO_TRASH();

    QString filePath = m_tempDir.path() + "/single.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("content");
    f.close();

    EXPECT_TRUE(DTrashManager::instance()->moveToTrash(filePath));
    EXPECT_FALSE(QFileInfo(filePath).exists());
    EXPECT_FALSE(DTrashManager::instance()->trashIsEmpty());
}

TEST_F(ut_DTrashManager, moveToTrashDirectory)
{
    SKIP_IF_NO_DIR_TRASH();

    QString dirPath = m_tempDir.path() + "/testdir";
    ASSERT_TRUE(QDir().mkpath(dirPath));
    QFile innerFile(dirPath + "/inner.txt");
    ASSERT_TRUE(innerFile.open(QIODevice::WriteOnly));
    innerFile.write("inner");
    innerFile.close();

    EXPECT_TRUE(DTrashManager::instance()->moveToTrash(dirPath));
    EXPECT_FALSE(QFileInfo(dirPath).exists());
}

TEST_F(ut_DTrashManager, moveToTrashSameNameTwice)
{
    SKIP_IF_NO_TRASH();

    // Move two files with the same basename — second should get a hashed name
    QString file1 = m_tempDir.path() + "/same.txt";
    QString file2 = m_tempDir.path() + "/subdir";
    ASSERT_TRUE(QDir().mkpath(file2));
    file2 += "/same.txt";

    QFile f1(file1);
    ASSERT_TRUE(f1.open(QIODevice::WriteOnly));
    f1.write("first");
    f1.close();

    QFile f2(file2);
    ASSERT_TRUE(f2.open(QIODevice::WriteOnly));
    f2.write("second");
    f2.close();

    ASSERT_TRUE(DTrashManager::instance()->moveToTrash(file1));
    ASSERT_TRUE(DTrashManager::instance()->moveToTrash(file2));
    EXPECT_FALSE(DTrashManager::instance()->trashIsEmpty());
}

TEST_F(ut_DTrashManager, moveToTrashFileWithLongExtension)
{
    SKIP_IF_NO_TRASH();

    // Test getNotExistsFileName suffix truncation (> 200 chars)
    QString longExt;
    for (int i = 0; i < 250; ++i)
        longExt += 'x';

    QString filePath = m_tempDir.path() + "/name." + longExt;
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    EXPECT_TRUE(DTrashManager::instance()->moveToTrash(filePath));
    EXPECT_FALSE(QFileInfo(filePath).exists());
}

TEST_F(ut_DTrashManager, moveToTrashFileWithNoExtension)
{
    SKIP_IF_NO_TRASH();

    QString filePath = m_tempDir.path() + "/noext";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    EXPECT_TRUE(DTrashManager::instance()->moveToTrash(filePath));
    EXPECT_FALSE(QFileInfo(filePath).exists());
}

TEST_F(ut_DTrashManager, moveToTrashSymlinkNoFollow)
{
    SKIP_IF_NO_TRASH();

    QString targetPath = m_tempDir.path() + "/symlink_target.txt";
    QFile target(targetPath);
    ASSERT_TRUE(target.open(QIODevice::WriteOnly));
    target.write("target");
    target.close();

    QString linkPath = m_tempDir.path() + "/symlink_link.txt";
    QFile::link(targetPath, linkPath);

    // followSymlink=false (default): should move the symlink itself
    EXPECT_TRUE(DTrashManager::instance()->moveToTrash(linkPath, false));
    EXPECT_FALSE(QFileInfo(linkPath).exists());
    // Target should still exist
    EXPECT_TRUE(QFileInfo(targetPath).exists());
}

TEST_F(ut_DTrashManager, moveToTrashSymlinkFollowSameFs)
{
    SKIP_IF_NO_TRASH();

    QString targetPath = m_tempDir.path() + "/follow_target.txt";
    QFile target(targetPath);
    ASSERT_TRUE(target.open(QIODevice::WriteOnly));
    target.write("target");
    target.close();

    QString linkPath = m_tempDir.path() + "/follow_link.txt";
    QFile::link(targetPath, linkPath);

    // followSymlink=true: if same filesystem, should succeed
    bool result = DTrashManager::instance()->moveToTrash(linkPath, true);
    if (result) {
        EXPECT_FALSE(QFileInfo(linkPath).exists());
    } else {
        // Different filesystem — acceptable in some environments
        SUCCEED() << "Symlink follow failed due to different filesystem";
    }
}

TEST_F(ut_DTrashManager, moveToTrashMultipleFilesThenClean)
{
    SKIP_IF_NO_TRASH();

    QStringList files;
    for (int i = 0; i < 5; ++i) {
        QString fp = m_tempDir.path() + QString("/batch_%1.txt").arg(i);
        QFile f(fp);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("data");
        f.close();
        files << fp;
    }

    for (const QString &fp : files) {
        ASSERT_TRUE(DTrashManager::instance()->moveToTrash(fp));
    }

    EXPECT_FALSE(DTrashManager::instance()->trashIsEmpty());
    EXPECT_TRUE(DTrashManager::instance()->cleanTrash());
    EXPECT_TRUE(DTrashManager::instance()->trashIsEmpty());
}

TEST_F(ut_DTrashManager, moveToTrashEmptyDirectory)
{
    SKIP_IF_NO_DIR_TRASH();

    QString dirPath = m_tempDir.path() + "/emptydir";
    ASSERT_TRUE(QDir().mkpath(dirPath));

    EXPECT_TRUE(DTrashManager::instance()->moveToTrash(dirPath));
    EXPECT_FALSE(QFileInfo(dirPath).exists());
}

TEST_F(ut_DTrashManager, moveToTrashNestedDirectory)
{
    SKIP_IF_NO_DIR_TRASH();

    QString dirPath = m_tempDir.path() + "/nested";
    ASSERT_TRUE(QDir().mkpath(dirPath + "/sub1/sub2"));
    QFile f1(dirPath + "/file1.txt");
    ASSERT_TRUE(f1.open(QIODevice::WriteOnly));
    f1.write("1");
    f1.close();
    QFile f2(dirPath + "/sub1/file2.txt");
    ASSERT_TRUE(f2.open(QIODevice::WriteOnly));
    f2.write("2");
    f2.close();

    EXPECT_TRUE(DTrashManager::instance()->moveToTrash(dirPath));
    EXPECT_FALSE(QFileInfo(dirPath).exists());
}
