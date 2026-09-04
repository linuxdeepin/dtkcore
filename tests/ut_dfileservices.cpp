// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/dfileservices.h"

#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QUrl>
#include <QList>

DCORE_USE_NAMESPACE

static bool fileManager1Available()
{
    QDBusInterface iface(QStringLiteral("org.freedesktop.FileManager1"),
                         QStringLiteral("/org/freedesktop/FileManager1"),
                         QStringLiteral("org.freedesktop.FileManager1"));
    return iface.isValid() && iface.connection().isConnected();
}

class ut_DFileServices : public testing::Test
{
protected:
    void SetUp() override
    {
        m_available = fileManager1Available();
    }
    void TearDown() override {}

    bool m_available = false;
};

TEST_F(ut_DFileServices, showFolderQString)
{
    bool result = DFileServices::showFolder(QStringLiteral("/tmp"));
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFolderQUrl)
{
    QUrl url = QUrl::fromLocalFile(QStringLiteral("/tmp"));
    bool result = DFileServices::showFolder(url);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFoldersQStringList)
{
    QList<QString> paths;
    paths << QStringLiteral("/tmp");
    bool result = DFileServices::showFolders(paths);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFoldersQUrlList)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QStringLiteral("/tmp"));
    bool result = DFileServices::showFolders(urls);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFileItemPropertieQString)
{
    bool result = DFileServices::showFileItemPropertie(QStringLiteral("/tmp"));
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFileItemPropertieQUrl)
{
    QUrl url = QUrl::fromLocalFile(QStringLiteral("/tmp"));
    bool result = DFileServices::showFileItemPropertie(url);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFileItemPropertiesQStringList)
{
    QList<QString> paths;
    paths << QStringLiteral("/tmp");
    bool result = DFileServices::showFileItemProperties(paths);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFileItemPropertiesQUrlList)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QStringLiteral("/tmp"));
    bool result = DFileServices::showFileItemProperties(urls);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFileItemQString)
{
    bool result = DFileServices::showFileItem(QStringLiteral("/tmp"));
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFileItemQUrl)
{
    QUrl url = QUrl::fromLocalFile(QStringLiteral("/tmp"));
    bool result = DFileServices::showFileItem(url);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFileItemsQStringList)
{
    QList<QString> paths;
    paths << QStringLiteral("/tmp");
    bool result = DFileServices::showFileItems(paths);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFileItemsQUrlList)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QStringLiteral("/tmp"));
    bool result = DFileServices::showFileItems(urls);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, trashQString)
{
    bool result = DFileServices::trash(QStringLiteral("/nonexistent_path_for_trash_test"));
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, trashQStringList)
{
    QList<QString> paths;
    paths << QStringLiteral("/nonexistent_path_for_trash_test");
    bool result = DFileServices::trash(paths);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, trashQUrl)
{
    QUrl url = QUrl::fromLocalFile(QStringLiteral("/nonexistent_path_for_trash_test"));
    bool result = DFileServices::trash(url);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, trashQUrlList)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QStringLiteral("/nonexistent_path_for_trash_test"));
    bool result = DFileServices::trash(urls);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, errorMessage)
{
    // When FileManager1 is unavailable, prior D-Bus calls set lastError() on the
    // static singleton, so errorMessage() may be non-empty — skip in that case.
    if (!m_available) GTEST_SKIP() << "FileManager1 not available";
    QString msg = DFileServices::errorMessage();
    EXPECT_TRUE(msg.isEmpty());
}

TEST_F(ut_DFileServices, showFolderWithStartupId)
{
    bool result = DFileServices::showFolder(QStringLiteral("/tmp"), QStringLiteral("test_startup_id"));
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFolderEmptyPath)
{
    bool result = DFileServices::showFolder(QString());
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

TEST_F(ut_DFileServices, showFoldersEmptyList)
{
    QList<QUrl> emptyUrls;
    bool result = DFileServices::showFolders(emptyUrls);
    if (m_available) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}
