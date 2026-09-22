// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filesystem/dcapmanager.h"

#include <gtest/gtest.h>
#include <QDir>
#include <QTemporaryDir>
#include <unistd.h>

DCORE_USE_NAMESPACE

class ut_DCapManager : public testing::Test
{
protected:
    void SetUp() override
    {
        manager = DCapManager::instance();
        originalPaths = manager->paths();
    }

    void TearDown() override
    {
        QStringList currentPaths = manager->paths();
        for (const QString &p : currentPaths) {
            if (!originalPaths.contains(p)) {
                manager->removePath(p);
            }
        }
    }

    DCapManager *manager = nullptr;
    QStringList originalPaths;
    QString testPath = QStringLiteral("/var/tmp/dcap_test_appendpath");
    QString basePath = QStringLiteral("/var/tmp/dcap_test_basepath");
    QStringList paths;
};

TEST_F(ut_DCapManager, instanceNotNull)
{
    EXPECT_NE(manager, nullptr);
}

TEST_F(ut_DCapManager, instanceSingleton)
{
    DCapManager *inst1 = DCapManager::instance();
    DCapManager *inst2 = DCapManager::instance();
    EXPECT_EQ(inst1, inst2);
}

TEST_F(ut_DCapManager, pathsNotEmpty)
{
    EXPECT_FALSE(manager->paths().isEmpty());
}

TEST_F(ut_DCapManager, appendPath)
{
    manager->appendPath(testPath);
    EXPECT_TRUE(manager->paths().contains(testPath));
}

TEST_F(ut_DCapManager, appendPathTrailingSeparator)
{
    QString pathWithSep = QStringLiteral("/var/tmp/dcap_test_trailing_sep/");
    QString cleanedPath = QStringLiteral("/var/tmp/dcap_test_trailing_sep");
    manager->appendPath(pathWithSep);
    EXPECT_TRUE(manager->paths().contains(cleanedPath));
}

TEST_F(ut_DCapManager, appendPathDuplicate)
{
    manager->appendPath(testPath);
    int countBefore = manager->paths().count(testPath);
    manager->appendPath(testPath);
    int countAfter = manager->paths().count(testPath);
    EXPECT_EQ(countBefore, countAfter);
}

TEST_F(ut_DCapManager, appendPathSubfileOfExisting)
{
    manager->appendPath(basePath);
    QString subPath = basePath + QStringLiteral("/subdir");
    manager->appendPath(subPath);
    EXPECT_FALSE(manager->paths().contains(subPath));
}

TEST_F(ut_DCapManager, appendPaths)
{
    paths << QStringLiteral("/var/tmp/dcap_test_multi1_unique");
    paths << QStringLiteral("/var/tmp/dcap_test_multi2_unique");
    manager->appendPaths(paths);
    EXPECT_TRUE(manager->paths().contains(paths[0]));
    EXPECT_TRUE(manager->paths().contains(paths[1]));
}

TEST_F(ut_DCapManager, appendPathsEmpty)
{
    int countBefore = manager->paths().size();
    manager->appendPaths(QStringList());
    EXPECT_EQ(manager->paths().size(), countBefore);
}

TEST_F(ut_DCapManager, removePath)
{
    manager->appendPath(testPath);
    ASSERT_TRUE(manager->paths().contains(testPath));
    manager->removePath(testPath);
    EXPECT_FALSE(manager->paths().contains(testPath));
}

TEST_F(ut_DCapManager, removePathTrailingSeparator)
{
    manager->appendPath(testPath);
    ASSERT_TRUE(manager->paths().contains(testPath));
    manager->removePath(testPath + QStringLiteral("/"));
    EXPECT_FALSE(manager->paths().contains(testPath));
}

TEST_F(ut_DCapManager, removePathNonExistent)
{
    QString testPath = QStringLiteral("/var/tmp/dcap_test_nonexistent_remove_unique");
    int countBefore = manager->paths().size();
    manager->removePath(testPath);
    EXPECT_EQ(manager->paths().size(), countBefore);
}

TEST_F(ut_DCapManager, removePaths)
{
    paths << QStringLiteral("/var/tmp/dcap_test_remove_multi1_unique");
    paths << QStringLiteral("/var/tmp/dcap_test_remove_multi2_unique");
    manager->appendPaths(paths);
    ASSERT_TRUE(manager->paths().contains(paths[0]));
    ASSERT_TRUE(manager->paths().contains(paths[1]));
    manager->removePaths(paths);
    EXPECT_FALSE(manager->paths().contains(paths[0]));
    EXPECT_FALSE(manager->paths().contains(paths[1]));
}

TEST_F(ut_DCapManager, removePathsEmpty)
{
    int countBefore = manager->paths().size();
    manager->removePaths(QStringList());
    EXPECT_EQ(manager->paths().size(), countBefore);
}

TEST_F(ut_DCapManager, appendAndRemoveCycle)
{
    manager->appendPath(testPath);
    EXPECT_TRUE(manager->paths().contains(testPath));
    manager->removePath(testPath);
    EXPECT_FALSE(manager->paths().contains(testPath));
    manager->appendPath(testPath);
    EXPECT_TRUE(manager->paths().contains(testPath));
    manager->removePath(testPath);
    EXPECT_FALSE(manager->paths().contains(testPath));
}

TEST_F(ut_DCapManager, appendEmptyPath)
{
    manager->appendPath(QString());
    SUCCEED();
}

TEST_F(ut_DCapManager, removeEmptyPath)
{
    manager->removePath(QString());
    SUCCEED();
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
TEST_F(ut_DCapManager, registerFileEngine)
{
    DCapManager::registerFileEngine();
    SUCCEED();
}

TEST_F(ut_DCapManager, unregisterFileEngine)
{
    DCapManager::unregisterFileEngine();
    SUCCEED();
}
#endif
