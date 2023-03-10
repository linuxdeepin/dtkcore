// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filesystem/dcapmanager.h"
#include "filesystem/dcapfile.h"

#include <gtest/gtest.h>
#include <QDir>
#include <QDir>
#include <QTemporaryDir>

DCORE_USE_NAMESPACE

#ifndef GTEST_SKIP
#define SKIP return GTEST_SUCCEED() << "Skip all tests"
#else
#define SKIP GTEST_SKIP() << "Skip all tests"
#endif

#define TMPCAP_PATH "/tmp/cap"

class ut_DCapFSFileEngine : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DCapManager *manager;
    QFile *file;
};

void ut_DCapFSFileEngine::SetUp()
{
     manager = DCapManager::instance();
     manager->removePath("/tmp");
     manager->appendPath(TMPCAP_PATH);

     file = new QFile(nullptr);
     QDir dir(TMPCAP_PATH);
     if (!dir.exists())
         ASSERT_TRUE(dir.mkdir(TMPCAP_PATH));
}

void ut_DCapFSFileEngine::TearDown()
{
    manager->appendPath("/tmp");
    delete file;
    QDir dir(TMPCAP_PATH);
    if (dir.exists())
        ASSERT_TRUE(dir.removeRecursively());
}

/*TEST_F(ut_DCapFSFileEngine, testSubDirCanReadWrite)
{
    manager->appendPath("/usr/share/");
    ASSERT_FALSE(DCapFSFileEngine("").canReadWrite("/tmp/usr/share/file0"));
    manager->removePath("/usr/share/");
}

TEST_F(ut_DCapFSFileEngine, testDCapFileOpenFile)
{
    file->setFileName("/tmp/file0");
    bool open = file->open(QIODevice::WriteOnly);
    ASSERT_FALSE(open);  // path `/tmp` has removed from setup.

    QDir dir(TMPCAP_PATH);
    ASSERT_TRUE(dir.exists());
    if (!dir.exists("subdir"))
        ASSERT_TRUE(dir.mkdir("subdir"));
    ASSERT_TRUE(dir.cd("subdir"));
    file->close();
    file->setFileName(dir.path() + "/file0");  // path: /tmp/etc/subdir/file0
    ASSERT_TRUE(file->open(QIODevice::WriteOnly));
}

TEST_F(ut_DCapFSFileEngine, testDCapFileRename)
{
    file->setFileName(TMPCAP_PATH"/file0");
    bool open = file->open(QIODevice::WriteOnly);
    ASSERT_TRUE(open);  // Default path contains the `/tmp` path.
    file->close();

    ASSERT_TRUE(file->rename(TMPCAP_PATH"/file1"));
    ASSERT_FALSE(file->rename("/tmp/file1"));

    QDir dir(TMPCAP_PATH);
    if (!dir.exists("subdir"))
        ASSERT_TRUE(dir.mkdir("subdir"));
    ASSERT_TRUE(file->rename(TMPCAP_PATH"/subdir/file0"));
}

TEST_F(ut_DCapFSFileEngine, testDCapFileRemove)
{
    file->setFileName(TMPCAP_PATH"/test");
    ASSERT_TRUE(file->open(QFile::WriteOnly));
    file->close();
    ASSERT_TRUE(file->exists());
    ASSERT_TRUE(file->remove());
    ASSERT_FALSE(file->exists());

    QDir dir(TMPCAP_PATH);
    if (!dir.exists("subdir"))
        ASSERT_TRUE(dir.mkdir("subdir"));

    manager->appendPath(TMPCAP_PATH"/subdir");  // create a new subdir file.
    ASSERT_FALSE(manager->paths().contains(TMPCAP_PATH"/subdir"));  // already has /tmp/cap
    QFile f(TMPCAP_PATH"/subdir/test");
    ASSERT_TRUE(f.open(QFile::WriteOnly));
    f.close();
    ASSERT_TRUE(f.exists());

    file->setFileName(TMPCAP_PATH"/subdir/test");
    ASSERT_TRUE(file->remove());

    manager->appendPath("/tmp");
    ASSERT_TRUE(manager->paths().contains("/tmp"));
    file->setFileName("/tmp/file0");
    bool open = file->open(QIODevice::WriteOnly);
    ASSERT_TRUE(open);
    manager->removePath("/tmp");
    ASSERT_FALSE(manager->paths().contains("/tmp"));
    ASSERT_FALSE(file->remove());
}

TEST_F(ut_DCapFSFileEngine, testDCapFileLink)
{
    file->setFileName(TMPCAP_PATH"/test");
    ASSERT_TRUE(file->open(QFile::WriteOnly));
    file->close();
    ASSERT_TRUE(file->exists());
    ASSERT_TRUE(file->link(TMPCAP_PATH"/test1"));
    ASSERT_FALSE(file->link("/tmp/test1"));

    QDir dir(TMPCAP_PATH);
    if (!dir.exists("subdir"))
        ASSERT_TRUE(dir.mkdir("subdir"));
    ASSERT_TRUE(file->link(TMPCAP_PATH"/subdir/test1"));

    ASSERT_TRUE(file->remove());  // clean the file.
}

TEST_F(ut_DCapFSFileEngine, testDCapFileCopy)
{
    file->setFileName(TMPCAP_PATH"/test");
    ASSERT_TRUE(file->open(QFile::WriteOnly));
    file->write("test");
    file->close();
    ASSERT_TRUE(file->exists());
    ASSERT_TRUE(file->copy(TMPCAP_PATH"/test2"));
    ASSERT_TRUE(file->copy("/tmp/test2"));
    ASSERT_FALSE(QFileInfo::exists(TMPCAP_PATH"/subdir/test2"));

    QDir dir(TMPCAP_PATH);
    if (!dir.exists("subdir"))
        ASSERT_TRUE(dir.mkdir("subdir"));
    ASSERT_TRUE(file->copy(TMPCAP_PATH"/subdir/test2"));
    ASSERT_TRUE(QFileInfo::exists(TMPCAP_PATH"/subdir/test2"));
}

TEST_F(ut_DCapFSFileEngine, testDCapFileResize)
{
    file->setFileName(TMPCAP_PATH"/test");
    ASSERT_TRUE(file->open(QFile::WriteOnly));
    file->write("test");
    file->close();
    ASSERT_TRUE(file->exists());
    ASSERT_TRUE(file->resize(10));
    ASSERT_EQ(file->size(), 10);

    QDir dir(TMPCAP_PATH);
    if (!dir.exists("subdir"))
        ASSERT_TRUE(dir.mkdir("subdir"));

    QFile f(TMPCAP_PATH"/subdir/test");
    ASSERT_TRUE(f.open(QFile::WriteOnly));
    f.write("test");
    f.close();
    ASSERT_TRUE(f.exists());

    file->setFileName(TMPCAP_PATH"/subdir/test");
    ASSERT_TRUE(file->resize(10));

    DCapManager::instance()->appendPath("/tmp");

    f.setFileName("/tmp/test");
    ASSERT_TRUE(f.open(QFile::WriteOnly));
    f.write("test");
    f.close();
    ASSERT_TRUE(f.exists());
    DCapManager::instance()->removePath("/tmp");

    file->setFileName("/tmp/test");
    ASSERT_FALSE(file->resize(10));
    file->remove();}

TEST_F(ut_DCapFSFileEngine, testDCapDirEntry)
{
    auto createFiles = [this](const QString & path, int count) {
        for (int i = 0; i < count; ++i) {
            file->setFileName(QString(path) + "/test" + QString::number(i));
            ASSERT_TRUE(file->open(QFile::WriteOnly));
            file->close();
            ASSERT_TRUE(file->exists());
        }
    };

    createFiles(TMPCAP_PATH, 10);
    QDir dir(TMPCAP_PATH);
    ASSERT_TRUE(dir.entryList(QDir::Files).length() == 10);

    if (!dir.exists("subdir"))
        ASSERT_TRUE(dir.mkdir("subdir"));

    dir.setPath(TMPCAP_PATH"/subdir");
    createFiles(TMPCAP_PATH"/subdir", 10);
    ASSERT_TRUE(dir.entryList(QDir::Files).length() == 10);
    manager->removePath(TMPCAP_PATH);
    ASSERT_TRUE(dir.entryList(QDir::Files).length() == 0);
}*/

TEST(ut_DCapFileAndDir, testDCapFileOpen)
{
    DCapFile file("/tmp/test0");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));  // Default, '/tmp' is allowable for writing.
    file.close();

    DCapManager::instance()->removePath("/tmp");
    ASSERT_FALSE(file.open(DCapFile::WriteOnly));
    DCapManager::instance()->appendPath("/tmp");
}

TEST(ut_DCapFileAndDir, testDCapFileOperation)
{
    auto CheckResult = [](bool result) {
        DCapFile file("/tmp/test0");
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
        ASSERT_EQ(file.rename("/tmp/test1"), result);
        ASSERT_EQ(DCapFile::rename(file.fileName(), "/tmp/test0"), result);

        file.setFileName("/tmp/test0");
        ASSERT_EQ(file.link("/tmp/test_link0"), result);
        ASSERT_EQ(DCapFile::link(file.fileName(), "/tmp/test_link1"), result);
        ASSERT_EQ(DCapFile::remove("/tmp/test_link0"), result);
        ASSERT_EQ(DCapFile::remove("/tmp/test_link1"), result);

        file.setFileName("/tmp/test0");
        ASSERT_EQ(file.copy("/tmp/test_copy0"), result);
        ASSERT_EQ(DCapFile::copy(file.fileName(), "/tmp/test_copy1"), result);
        ASSERT_EQ(DCapFile::remove("/tmp/test_copy0"), result);
        ASSERT_EQ(DCapFile::remove("/tmp/test_copy1"), result);

        ASSERT_EQ(file.resize(10), result);
        ASSERT_EQ(file.size() == 10, result);
        ASSERT_EQ(DCapFile::resize(file.fileName(), 5), result);
        ASSERT_EQ(file.size() == 5, result);
        ASSERT_EQ(file.remove(), result);
    };

    CheckResult(true);
    DCapManager::instance()->removePath("/tmp");
    CheckResult(false);
    DCapManager::instance()->appendPath("/tmp");
}

TEST(ut_DCapFileAndDir, testDCapDirOperation)
{
    DCapDir dir("/tmp");
    ASSERT_TRUE(dir.exists());
    ASSERT_FALSE(dir.entryList().isEmpty());
    ASSERT_FALSE(dir.entryInfoList().isEmpty());

    DCapFile file(dir.path() + "/test0");
    ASSERT_TRUE(file.open(DCapFile::WriteOnly));
    file.close();
    ASSERT_TRUE(dir.exists("test0"));

    ASSERT_TRUE(dir.mkdir("cap"));
    ASSERT_TRUE(dir.exists("cap"));
    ASSERT_TRUE(dir.rmdir("cap"));
    ASSERT_FALSE(dir.exists("cap"));

    dir.mkdir("cap");
    ASSERT_TRUE(dir.cd("cap"));
    ASSERT_TRUE(dir.exists());
    DCapManager::instance()->removePath("/tmp");
    DCapManager::instance()->appendPath(dir.path());
    ASSERT_FALSE(dir.cd(".."));
    dir.setPath("/tmp");

    ASSERT_TRUE(dir.entryList().isEmpty());
    ASSERT_TRUE(dir.entryInfoList().isEmpty());
    ASSERT_TRUE(dir.exists("cap"));
    ASSERT_FALSE(dir.remove("test0"));
    ASSERT_FALSE(dir.rename("test0", "test1"));

    ASSERT_TRUE(dir.mkpath(TMPCAP_PATH"/subdir"));
    ASSERT_TRUE(dir.rmpath(TMPCAP_PATH"/subdir"));

    DCapManager::instance()->appendPath("/tmp");
    ASSERT_TRUE(dir.rename("test0", "test1"));
    ASSERT_TRUE(dir.remove("test1"));
}
