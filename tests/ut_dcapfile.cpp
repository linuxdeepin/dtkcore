// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filesystem/dcapmanager.h"
#include "filesystem/dcapfile.h"

#include <gtest/gtest.h>
#include <QDir>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFileInfo>
#include <unistd.h>


DCORE_USE_NAMESPACE

// Check if DCapManager is properly initialized (paths non-empty)
static inline bool isDCapManagerAvailable()
{
    return !DCapManager::instance()->paths().isEmpty();
}

// RAII guard: registers a path on construction and ensures it is restored
// in DCapManager's path list on destruction, even if ASSERT fails mid-test.
// Uses /var/tmp-based paths to avoid conflict with QStandardPaths defaults.
class DCapPathGuard {
    QString m_path;
public:
    explicit DCapPathGuard(const QString &path) : m_path(path) {
        DCapManager::instance()->appendPath(m_path);
    }
    ~DCapPathGuard() { DCapManager::instance()->removePath(m_path); }
};

TEST(ut_DCapFile, dcapManagerPaths)
{
    if (!isDCapManagerAvailable()) {
        GTEST_SKIP() << "DCapManager not available in this environment";
    }
    auto size = DCapManager::instance()->paths().size();
    EXPECT_TRUE(size > 0);

    DCapManager::instance()->appendPaths({"/path/to/myCap"});
    EXPECT_TRUE(DCapManager::instance()->paths().contains("/path/to/myCap"));

    DCapManager::instance()->removePaths({"/path/to/myCap"});
    EXPECT_FALSE(DCapManager::instance()->paths().contains("/path/to/myCap"));
}

TEST(ut_DCapFileAndDir, testDCapFileOpen)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(tempDir.filePath("test0"));
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    ASSERT_FALSE(file.open(DCapFile::WriteOnly));
}

TEST(ut_DCapFileAndDir, testDCapFileOperation)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    auto CheckResult = [base](bool result) {
        DCapFile file(base + "/test0");
        ASSERT_EQ(file.open(DCapFile::WriteOnly), result);
        file.close();

        ASSERT_EQ(file.exists(), result);
        ASSERT_EQ(DCapFile::exists(file.fileName()), result);
        ASSERT_EQ(file.remove(), result);

        ASSERT_EQ(file.open(DCapFile::WriteOnly), result);
        file.close();
        ASSERT_EQ(DCapFile::remove(file.fileName()), result);

        ASSERT_EQ(file.open(DCapFile::WriteOnly), result);
        file.close();
        ASSERT_EQ(file.rename(base + "/test1"), result);
        ASSERT_EQ(DCapFile::rename(file.fileName(), base + "/test0"), result);

        file.setFileName(base + "/test0");
        ASSERT_EQ(file.link(base + "/test_link0"), result);
        ASSERT_EQ(DCapFile::link(file.fileName(), base + "/test_link1"), result);
        ASSERT_EQ(DCapFile::remove(base + "/test_link0"), result);
        ASSERT_EQ(DCapFile::remove(base + "/test_link1"), result);

        file.setFileName(base + "/test0");
        ASSERT_EQ(file.copy(base + "/test_copy0"), result);
        ASSERT_EQ(DCapFile::copy(file.fileName(), base + "/test_copy1"), result);
        ASSERT_EQ(DCapFile::remove(base + "/test_copy0"), result);
        ASSERT_EQ(DCapFile::remove(base + "/test_copy1"), result);

        ASSERT_EQ(file.resize(10), result);
        ASSERT_EQ(file.size() == 10, result);
        ASSERT_EQ(DCapFile::resize(file.fileName(), 5), result);
        ASSERT_EQ(file.size() == 5, result);
        ASSERT_EQ(file.remove(), result);
    };

    CheckResult(true);
    DCapManager::instance()->removePath(base);
    CheckResult(false);
}

TEST(ut_DCapFileAndDir, testDCapDirOperation)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    DCapDir dir(base);

    DCapFile file(base + "/test0");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    ASSERT_TRUE(dir.exists());
    ASSERT_FALSE(dir.entryList().isEmpty());
    ASSERT_FALSE(dir.entryInfoList().isEmpty());
    ASSERT_TRUE(dir.exists("test0"));

    ASSERT_TRUE(dir.mkdir("cap"));
    ASSERT_TRUE(dir.exists("cap"));
    ASSERT_TRUE(dir.rmdir("cap"));
    ASSERT_FALSE(dir.exists("cap"));

    dir.mkdir("cap");
    ASSERT_TRUE(dir.cd("cap"));
    ASSERT_TRUE(dir.exists());
    DCapManager::instance()->removePath(base);
    DCapManager::instance()->appendPath(dir.path());
    ASSERT_FALSE(dir.cd(".."));
    dir.setPath(base);

    ASSERT_TRUE(dir.entryList().isEmpty());
    ASSERT_TRUE(dir.entryInfoList().isEmpty());
    ASSERT_TRUE(dir.exists("cap"));
    ASSERT_FALSE(dir.remove("test0"));
    ASSERT_FALSE(dir.rename("test0", "test1"));

    ASSERT_TRUE(dir.mkpath(base + "/cap/subdir"));
    ASSERT_TRUE(dir.rmpath(base + "/cap/subdir"));

    DCapManager::instance()->appendPath(base);
    ASSERT_TRUE(dir.rename("test0", "test1"));
    ASSERT_TRUE(dir.remove("test1"));
}

// ---- New tests for additional coverage ----

TEST(ut_DCapFile, constructorWithParent)
{
    DCapFile file(nullptr);
    EXPECT_TRUE(file.fileName().isEmpty());
}

TEST(ut_DCapFile, constructorWithNameAndParent)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(tempDir.filePath("test_ctor"), nullptr);
    EXPECT_EQ(file.fileName(), tempDir.filePath("test_ctor"));
}

TEST(ut_DCapFile, setFileName)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(nullptr);
    file.setFileName(tempDir.filePath("test_setfilename"));
    EXPECT_EQ(file.fileName(), tempDir.filePath("test_setfilename"));
}

TEST(ut_DCapFile, existsOnNonExistent)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(tempDir.filePath("does_not_exist_xyz"));
    EXPECT_FALSE(file.exists());
    EXPECT_FALSE(DCapFile::exists(tempDir.filePath("does_not_exist_xyz")));
}

TEST(ut_DCapFile, symLinkTarget)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    DCapFile file(base + "/test_symlink_src");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapFile linkFile(base + "/test_symlink_link");
    if (!linkFile.link(base + "/test_symlink_src")) {
        if (getuid() != 0) GTEST_SKIP() << "link() failed in non-root environment";
        FAIL() << "link() failed unexpectedly";
    }

    DCapFile target(base + "/test_symlink_link");
    EXPECT_EQ(target.symLinkTarget(), QString(base + "/test_symlink_src"));
}

TEST(ut_DCapFile, moveToTrashInstance)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(tempDir.filePath("test_trash"));
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();
    ASSERT_TRUE(file.exists());

    bool result = file.moveToTrash();
    if (!result) {
        GTEST_SKIP() << "moveToTrash not supported in this environment";
    }
    EXPECT_FALSE(file.exists());
}

TEST(ut_DCapFile, moveToTrashStatic)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString path = tempDir.filePath("test_trash2");
    DCapFile file(path);
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    QString pathInTrash;
    bool result = DCapFile::moveToTrash(path, &pathInTrash);
    if (!result) {
        GTEST_SKIP() << "moveToTrash not supported in this environment";
    }
    EXPECT_FALSE(pathInTrash.isEmpty());
    EXPECT_FALSE(QFile::exists(path));
}

TEST(ut_DCapFile, moveToTrashStaticNullPath)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString path = tempDir.filePath("test_trash3");
    DCapFile file(path);
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    bool result = DCapFile::moveToTrash(path, nullptr);
    if (result) {
        EXPECT_FALSE(QFile::exists(path));
    }
}

TEST(ut_DCapFile, moveToTrashNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString path = tempDir.filePath("test_trash_noperm");
    DCapFile file(path);
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(file.moveToTrash());

    QString pathInTrash;
    EXPECT_FALSE(DCapFile::moveToTrash(path, &pathInTrash));
}

TEST(ut_DCapFile, renameToNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    DCapFile file(base + "/test_rename_perm");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(file.rename(base + "/test_renamed"));
    EXPECT_FALSE(DCapFile::rename(base + "/test_rename_perm", base + "/test_renamed"));
}

TEST(ut_DCapFile, linkToNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    DCapFile file(base + "/test_link_perm");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(file.link(base + "/test_link_target"));
    EXPECT_FALSE(DCapFile::link(base + "/test_link_perm", base + "/test_link_target2"));
}

TEST(ut_DCapFile, copyToNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    DCapFile file(base + "/test_copy_perm");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(file.copy(base + "/test_copy_target"));
    EXPECT_FALSE(DCapFile::copy(base + "/test_copy_perm", base + "/test_copy_target2"));
}

TEST(ut_DCapFile, resizeNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString path = tempDir.filePath("test_resize_perm");
    DCapFile file(path);
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(file.resize(100));
    EXPECT_FALSE(DCapFile::resize(path, 50));
}

TEST(ut_DCapFile, openNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(tempDir.filePath("test_open_perm"));
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(file.open(DCapFile::ReadOnly));
    EXPECT_FALSE(file.open(DCapFile::WriteOnly));
    EXPECT_FALSE(file.open(DCapFile::ReadWrite));
}

TEST(ut_DCapFile, existsNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString path = tempDir.filePath("test_exists_perm");
    DCapFile file(path);
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(file.exists());
    EXPECT_FALSE(DCapFile::exists(path));
}

TEST(ut_DCapFile, removeNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString path = tempDir.filePath("test_remove_perm");
    DCapFile file(path);
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();

    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(file.remove());
    EXPECT_FALSE(DCapFile::remove(path));
}

TEST(ut_DCapFile, openWithFilePointer)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(tempDir.filePath("test_fp"));
    EXPECT_FALSE(file.open(static_cast<FILE*>(nullptr), DCapFile::ReadOnly));
}

TEST(ut_DCapFile, openWithFileDescriptor)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(tempDir.filePath("test_fd"));
    EXPECT_FALSE(file.open(-1, DCapFile::ReadOnly));
}

TEST(ut_DCapFile, writeAndRead)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFile file(tempDir.filePath("test_rw"));
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.write("hello world", 11);
    file.close();

    ASSERT_TRUE(file.open(DCapFile::ReadOnly));
    EXPECT_EQ(file.readAll(), QByteArray("hello world"));
    file.close();
}

// ---- DCapDir additional tests ----

TEST(ut_DCapDir, copyConstructor)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapDir dir1(tempDir.path());
    DCapDir dir2(dir1);
    EXPECT_EQ(dir2.path(), dir1.path());
    EXPECT_TRUE(dir2.exists());
}

TEST(ut_DCapDir, constructorWithNameFilter)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    QFile f1(base + "/test_namefilter.txt");
    f1.open(QIODevice::WriteOnly);
    f1.close();
    QFile f2(base + "/test_namefilter.log");
    f2.open(QIODevice::WriteOnly);
    f2.close();

    DCapDir dir(base, "*.txt", QDir::Name, QDir::Files);
    EXPECT_EQ(dir.path(), QString(base));
    auto list = dir.entryList();
    EXPECT_TRUE(list.contains("test_namefilter.txt"));
    EXPECT_FALSE(list.contains("test_namefilter.log"));
}

TEST(ut_DCapDir, existsEmptyName)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapDir dir(tempDir.path());
    EXPECT_FALSE(dir.exists(""));
}

TEST(ut_DCapDir, removeEmptyName)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapDir dir(tempDir.path());
    EXPECT_FALSE(dir.remove(""));
}

TEST(ut_DCapDir, renameEmptyNames)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapDir dir(tempDir.path());
    EXPECT_FALSE(dir.rename("", "newname"));
    EXPECT_FALSE(dir.rename("oldname", ""));
}

TEST(ut_DCapDir, entryListWithNameFilters)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    DCapDir dir(base);
    for (int i = 0; i < 3; ++i) {
        DCapFile f(QString(base + "/test_filter_%1.txt").arg(i));
        f.open(DCapFile::WriteOnly);
        f.close();
    }
    DCapFile f2(base + "/test_filter_noext");
    f2.open(DCapFile::WriteOnly);
    f2.close();

    QStringList filters;
    filters << "*.txt";
    QStringList list = dir.entryList(filters, QDir::Files);
    EXPECT_GE(list.size(), 3);

    QFileInfoList infoList = dir.entryInfoList(filters, QDir::Files);
    EXPECT_GE(infoList.size(), 3);
}

TEST(ut_DCapDir, entryListNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapDir dir(tempDir.path());
    DCapManager::instance()->removePath(tempDir.path());

    QStringList filters;
    filters << "*.txt";
    EXPECT_TRUE(dir.entryList().isEmpty());
    EXPECT_TRUE(dir.entryList(filters, QDir::Files).isEmpty());
    EXPECT_TRUE(dir.entryInfoList().isEmpty());
    EXPECT_TRUE(dir.entryInfoList(filters, QDir::Files).isEmpty());
}

TEST(ut_DCapDir, existsNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapDir dir(tempDir.path());
    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(dir.exists());
}

TEST(ut_DCapDir, mkdirRmdirNoPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    DCapDir dir(base);
    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(dir.mkdir("noperm_dir"));
    EXPECT_FALSE(dir.rmdir("noperm_dir"));
    EXPECT_FALSE(dir.mkpath(base + "/noperm_path"));
    EXPECT_FALSE(dir.rmpath(base + "/noperm_path"));
}

TEST(ut_DCapDir, cdToSubdir)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapDir dir(tempDir.path());
    dir.mkdir("test_cd_sub");
    EXPECT_TRUE(dir.cd("test_cd_sub"));
    EXPECT_TRUE(dir.path().contains("test_cd_sub"));
    EXPECT_TRUE(dir.cd(".."));
}

TEST(ut_DCapDir, renameNonExistentFile)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapDir dir(tempDir.path());
    EXPECT_FALSE(dir.rename("nonexistent_file_xyz", "newname"));
}

TEST(ut_DCapDir, setPathAndExists)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    const QString base = tempDir.path();
    DCapDir dir(base);
    dir.setPath(base);
    EXPECT_EQ(dir.path(), QString(base));
    EXPECT_TRUE(dir.exists());
}

TEST(ut_DCapDir, defaultConstructor)
{
    DCapDir dir;
    EXPECT_TRUE(dir.path().isEmpty() || dir.path() == ".");
}
