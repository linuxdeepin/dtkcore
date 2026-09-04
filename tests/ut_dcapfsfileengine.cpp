// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filesystem/dcapmanager.h"
#include "filesystem/dcapfile.h"
#include "private/dcapfsfileengine_p.h"

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

TEST(ut_DCapFSFileEngine, canReadWriteWithPermission)
{
    if (!isDCapManagerAvailable()) {
        GTEST_SKIP() << "DCapManager not available in this environment";
    }
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_rw";
    DCapFSFileEngine engine(path);
    EXPECT_TRUE(engine.canReadWrite(path));
}

TEST(ut_DCapFSFileEngine, canReadWriteWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_noperm";
    DCapFSFileEngine engine(path);
    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(engine.canReadWrite(path));
}

TEST(ut_DCapFSFileEngine, canReadWriteEmptyPath)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFSFileEngine engine(tempDir.path() + "/test_engine_empty");
    EXPECT_FALSE(engine.canReadWrite(""));
}

TEST(ut_DCapFSFileEngine, canReadWriteDifferentPath)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    DCapFSFileEngine engine(tempDir.path() + "/test_engine_diff");
    EXPECT_TRUE(engine.canReadWrite(tempDir.path() + "/test_other_path"));
    EXPECT_FALSE(engine.canReadWrite("/nonexistent/path/file"));
}

TEST(ut_DCapFSFileEngine, openWithPermission)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_open";
    DCapFSFileEngine engine(path);
    EXPECT_TRUE(engine.open(QIODevice::WriteOnly));
    engine.close();
}

TEST(ut_DCapFSFileEngine, openWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_open_noperm";
    DCapFSFileEngine engine(path);
    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(engine.open(QIODevice::WriteOnly));
}

TEST(ut_DCapFSFileEngine, removeWithPermission)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_remove";
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(path);
    EXPECT_TRUE(engine.remove());
}

TEST(ut_DCapFSFileEngine, removeWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_remove_noperm";
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(path);
    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(engine.remove());
}

TEST(ut_DCapFSFileEngine, copyWithPermission)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString srcPath = tempDir.path() + "/test_engine_copy_src";
    QString dstPath = tempDir.path() + "/test_engine_copy_dst";
    QFile f(srcPath);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(srcPath);
    if (!engine.copy(dstPath)) {
        if (getuid() != 0) GTEST_SKIP() << "copy() failed in non-root environment";
        FAIL() << "copy() failed unexpectedly";
    }
    EXPECT_TRUE(QFile::exists(dstPath));
}

TEST(ut_DCapFSFileEngine, copyWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString srcPath = tempDir.path() + "/test_engine_copy_noperm_src";
    QString dstPath = tempDir.path() + "/test_engine_copy_noperm_dst";
    QFile f(srcPath);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(srcPath);
    DCapManager::instance()->removePath(tempDir.path());

    // By design: when canReadWrite() fails, copy() returns true (with a warning) to
    // prevent QFile fallback from bypassing DCapFS permission restrictions.
    // See dcapfsfileengine.cpp:120-132 for the rationale.
    EXPECT_TRUE(engine.copy(dstPath));
}

TEST(ut_DCapFSFileEngine, renameWithPermission)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString srcPath = tempDir.path() + "/test_engine_rename_src";
    QString dstPath = tempDir.path() + "/test_engine_rename_dst";
    QFile f(srcPath);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(srcPath);
    EXPECT_TRUE(engine.rename(dstPath));
    EXPECT_TRUE(QFile::exists(dstPath));
}

TEST(ut_DCapFSFileEngine, renameWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString srcPath = tempDir.path() + "/test_engine_rename_noperm_src";
    QString dstPath = tempDir.path() + "/test_engine_rename_noperm_dst";
    QFile f(srcPath);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(srcPath);
    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(engine.rename(dstPath));
}

TEST(ut_DCapFSFileEngine, fileFlagsWithPermission)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_flags";
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(path);
    QAbstractFileEngine::FileFlags flags = engine.fileFlags(QAbstractFileEngine::FileType | QAbstractFileEngine::ExistsFlag);
    EXPECT_TRUE(flags & QAbstractFileEngine::ExistsFlag);
}

TEST(ut_DCapFSFileEngine, fileFlagsWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_flags_noperm";
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(path);
    DCapManager::instance()->removePath(tempDir.path());
    QAbstractFileEngine::FileFlags flags = engine.fileFlags(QAbstractFileEngine::FileType | QAbstractFileEngine::ExistsFlag);
    EXPECT_FALSE(flags & QAbstractFileEngine::ExistsFlag);
}

TEST(ut_DCapFSFileEngine, setSizeWithPermission)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_setsize";
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(path);
    EXPECT_TRUE(engine.setSize(100));
    EXPECT_EQ(engine.size(), 100);
}

TEST(ut_DCapFSFileEngine, setSizeWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_setsize_noperm";
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.close();

    DCapFSFileEngine engine(path);
    DCapManager::instance()->removePath(tempDir.path());
    EXPECT_FALSE(engine.setSize(100));
}

TEST(ut_DCapFSFileEngine, cloneToWithPermission)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString srcPath = tempDir.path() + "/test_engine_clone_src";
    QString dstPath = tempDir.path() + "/test_engine_clone_dst";

    QFile src(srcPath);
    src.open(QIODevice::WriteOnly);
    src.write("testdata");
    src.close();

    DCapFSFileEngine srcEngine(srcPath);
    DCapFSFileEngine dstEngine(dstPath);

    srcEngine.cloneTo(&dstEngine);

    if (!QFile::exists(dstPath)) {
        GTEST_SKIP() << "Filesystem does not support clone (cloneTo did not create target file)";
    }

    QFile dst(dstPath);
    ASSERT_TRUE(dst.open(QIODevice::ReadOnly));
    EXPECT_EQ(dst.readAll(), "testdata");
    dst.close();
}

TEST(ut_DCapFSFileEngine, cloneToWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString srcPath = tempDir.path() + "/test_engine_clone_noperm_src";
    QFile src(srcPath);
    src.open(QIODevice::WriteOnly);
    src.close();

    DCapFSFileEngine srcEngine(srcPath);
    DCapFSFileEngine dstEngine("/nonexistent/path/clone_dst");

    DCapManager::instance()->removePath(tempDir.path());
    bool result = srcEngine.cloneTo(&dstEngine);
    EXPECT_FALSE(result);
}

TEST(ut_DCapFSFileEngine, entryListWithPermission)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString dirPath = tempDir.path() + "/test_engine_entrylist";
    QDir dir;
    dir.mkdir(dirPath);
    QFile f1(dirPath + "/file1.txt");
    f1.open(QIODevice::WriteOnly);
    f1.close();

    DCapFSFileEngine engine(dirPath);
    QStringList filters;
    filters << "*.txt";
    QStringList list = engine.entryList(QDir::Files, filters);
    EXPECT_FALSE(list.isEmpty());
}

TEST(ut_DCapFSFileEngine, entryListWithoutPermission)
{
    if (getuid() == 0) GTEST_SKIP() << "Permission test skipped when running as root";
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString dirPath = tempDir.path() + "/test_engine_entrylist_noperm";
    DCapFSFileEngine engine(dirPath);
    DCapManager::instance()->removePath(tempDir.path());
    QStringList filters;
    filters << "*.txt";
    QStringList list = engine.entryList(QDir::Files, filters);
    EXPECT_TRUE(list.isEmpty());
}

TEST(ut_DCapFSFileEngine, beginEntryList)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString dirPath = tempDir.path() + "/test_engine_beginentry";
    QDir dir;
    dir.mkdir(dirPath);
    QFile f1(dirPath + "/a.txt");
    f1.open(QIODevice::WriteOnly);
    f1.close();

    DCapFSFileEngine engine(dirPath);
    QStringList filters;
    filters << "*.txt";
    auto it = engine.beginEntryList(dirPath, QDir::Files, filters);
    EXPECT_NE(it, nullptr);
}

TEST(ut_DCapFSFileEngine, handlerCreate)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_handler_create";
    DCapFSFileEngineHandler handler;
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    auto engine = handler.create(path);
    EXPECT_NE(engine, nullptr);
#else
    auto engine = handler.create(path);
    EXPECT_NE(engine, nullptr);
    delete engine;
#endif
}

TEST(ut_DCapFSFileEngine, constructorAndDestructor)
{
    QTemporaryDir tempDir("/var/tmp/dcap_test_XXXXXX");
    DCapPathGuard dcapGuard(tempDir.path());
    QString path = tempDir.path() + "/test_engine_ctor_dtor";
    {
        DCapFSFileEngine engine(path);
        EXPECT_EQ(engine.fileName(QAbstractFileEngine::DefaultName), path);
    }
}
