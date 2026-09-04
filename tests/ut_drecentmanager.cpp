// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QFile>
#include <QDomDocument>
#include <QUrl>
#include <QDir>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QTextStream>

#include "util/drecentmanager.h"

DCORE_USE_NAMESPACE

static const QString RecentPath = QDir::homePath() + "/.local/share/recently-used.xbel";

class ut_DRecentManager: public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    QString m_backupPath;
    bool m_hasBackup = false;
    QString m_testFile;

    bool bookmarkExists(const QString &uri);
    int bookmarkCount();
    int getAppCount(const QString &uri, const QString &appName, const QString &appExec);
    int countApplications(const QString &uri);
};

void ut_DRecentManager::SetUp()
{
    QDir().mkpath(QFileInfo(RecentPath).absolutePath());

    if (QFile::exists(RecentPath)) {
        m_backupPath = RecentPath + ".bak.ut_test";
        QFile::rename(RecentPath, m_backupPath);
        m_hasBackup = true;
    }

    QTemporaryFile tmp;
    tmp.setAutoRemove(false);
    tmp.open();
    tmp.write("hello world");
    tmp.close();
    m_testFile = tmp.fileName();
}

void ut_DRecentManager::TearDown()
{
    if (QFile::exists(RecentPath)) {
        QFile::remove(RecentPath);
    }
    if (m_hasBackup) {
        QFile::rename(m_backupPath, RecentPath);
    }
    if (QFile::exists(m_testFile)) {
        QFile::remove(m_testFile);
    }
}

bool ut_DRecentManager::bookmarkExists(const QString &uri)
{
    QFile file(RecentPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomNodeList nodeList = rootEle.elementsByTagName("bookmark");
    QUrl url = QUrl::fromLocalFile(uri);
    for (int i = 0; i < nodeList.size(); ++i) {
        const QString fileUrl = nodeList.at(i).toElement().attribute("href");
        if (fileUrl == url.toEncoded(QUrl::FullyDecoded)) {
            return true;
        }
    }
    return false;
}

int ut_DRecentManager::bookmarkCount()
{
    QFile file(RecentPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return 0;
    }
    file.close();
    return doc.documentElement().elementsByTagName("bookmark").size();
}

int ut_DRecentManager::getAppCount(const QString &uri, const QString &appName, const QString &appExec)
{
    QFile file(RecentPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return -1;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomNodeList nodeList = rootEle.elementsByTagName("bookmark");
    QUrl url = QUrl::fromLocalFile(uri);
    for (int i = 0; i < nodeList.size(); ++i) {
        const QString fileUrl = nodeList.at(i).toElement().attribute("href");
        if (fileUrl == url.toEncoded(QUrl::FullyDecoded)) {
            QDomElement bookmarkEle = nodeList.at(i).toElement();
            QDomNodeList appList = bookmarkEle.elementsByTagName("bookmark:application");
            for (int j = 0; j < appList.size(); ++j) {
                QDomElement appEle = appList.at(j).toElement();
                if (appEle.attribute("name") == appName &&
                    appEle.attribute("exec") == appExec) {
                    return appEle.attribute("count").toInt();
                }
            }
        }
    }
    return -1;
}

int ut_DRecentManager::countApplications(const QString &uri)
{
    QFile file(RecentPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return 0;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomNodeList nodeList = rootEle.elementsByTagName("bookmark");
    QUrl url = QUrl::fromLocalFile(uri);
    for (int i = 0; i < nodeList.size(); ++i) {
        const QString fileUrl = nodeList.at(i).toElement().attribute("href");
        if (fileUrl == url.toEncoded(QUrl::FullyDecoded)) {
            return nodeList.at(i).toElement()
                .elementsByTagName("bookmark:application").size();
        }
    }
    return 0;
}

TEST_F(ut_DRecentManager, addItemEmptyUri)
{
    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "text/plain";
    EXPECT_FALSE(DRecentManager::addItem("", data));
}

TEST_F(ut_DRecentManager, addItemNonExistentFile)
{
    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "text/plain";
    EXPECT_FALSE(DRecentManager::addItem("/tmp/nonexistent_file_12345678.txt", data));
}

TEST_F(ut_DRecentManager, addItemValidFile)
{
    DRecentData data;
    data.appExec = "deepin-editor";
    data.appName = "Deepin Editor";
    data.mimeType = "text/plain";
    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    EXPECT_TRUE(bookmarkExists(m_testFile));
}

TEST_F(ut_DRecentManager, addItemAutoDetectMimeType)
{
    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "";
    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));

    QFile file(RecentPath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(&file));
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomNodeList nodeList = rootEle.elementsByTagName("bookmark");
    QUrl url = QUrl::fromLocalFile(m_testFile);
    for (int i = 0; i < nodeList.size(); ++i) {
        const QString fileUrl = nodeList.at(i).toElement().attribute("href");
        if (fileUrl == url.toEncoded(QUrl::FullyDecoded)) {
            QDomNodeList mimeList = nodeList.at(i).toElement()
                .elementsByTagName("mime:mime-type");
            ASSERT_GT(mimeList.size(), 0);
            QString detectedType = mimeList.at(0).toElement().attribute("type");
            EXPECT_FALSE(detectedType.isEmpty());
            EXPECT_EQ(detectedType, QMimeDatabase().mimeTypeForFile(m_testFile).name());
        }
    }
}

TEST_F(ut_DRecentManager, addItemDuplicateIncrementsCount)
{
    DRecentData data;
    data.appExec = "deepin-editor";
    data.appName = "Deepin Editor";
    data.mimeType = "text/plain";

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    EXPECT_EQ(getAppCount(m_testFile, "Deepin Editor", "deepin-editor"), 1);

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    EXPECT_EQ(getAppCount(m_testFile, "Deepin Editor", "deepin-editor"), 2);

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    EXPECT_EQ(getAppCount(m_testFile, "Deepin Editor", "deepin-editor"), 3);
}

TEST_F(ut_DRecentManager, addItemSameFileDifferentApp)
{
    DRecentData data1;
    data1.appExec = "deepin-editor";
    data1.appName = "Deepin Editor";
    data1.mimeType = "text/plain";

    DRecentData data2;
    data2.appExec = "deepin-image-viewer";
    data2.appName = "Deepin Image Viewer";
    data2.mimeType = "text/plain";

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data1));
    EXPECT_EQ(countApplications(m_testFile), 1);

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data2));
    EXPECT_EQ(countApplications(m_testFile), 2);

    EXPECT_EQ(getAppCount(m_testFile, "Deepin Editor", "deepin-editor"), 1);
    EXPECT_EQ(getAppCount(m_testFile, "Deepin Image Viewer", "deepin-image-viewer"), 1);
}

TEST_F(ut_DRecentManager, addItemMultipleFiles)
{
    QTemporaryFile tmp2;
    tmp2.setAutoRemove(false);
    tmp2.open();
    tmp2.write("second file");
    tmp2.close();

    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "text/plain";

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    EXPECT_TRUE(DRecentManager::addItem(tmp2.fileName(), data));
    EXPECT_EQ(bookmarkCount(), 2);
    EXPECT_TRUE(bookmarkExists(m_testFile));
    EXPECT_TRUE(bookmarkExists(tmp2.fileName()));

    QFile::remove(tmp2.fileName());
}

TEST_F(ut_DRecentManager, removeItem)
{
    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "text/plain";

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    EXPECT_TRUE(bookmarkExists(m_testFile));

    QString encodedUrl = QUrl::fromLocalFile(m_testFile).toEncoded(QUrl::FullyDecoded);
    DRecentManager::removeItem(encodedUrl);
    EXPECT_FALSE(bookmarkExists(m_testFile));
}

TEST_F(ut_DRecentManager, removeItemNonExistent)
{
    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "text/plain";

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    int countBefore = bookmarkCount();

    DRecentManager::removeItem("/tmp/nonexistent_file_87654321.txt");
    EXPECT_EQ(bookmarkCount(), countBefore);
}

TEST_F(ut_DRecentManager, removeItems)
{
    QTemporaryFile tmp2;
    tmp2.setAutoRemove(false);
    tmp2.open();
    tmp2.write("second file");
    tmp2.close();

    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "text/plain";

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    EXPECT_TRUE(DRecentManager::addItem(tmp2.fileName(), data));
    EXPECT_EQ(bookmarkCount(), 2);

    QStringList list;
    list << QUrl::fromLocalFile(m_testFile).toEncoded(QUrl::FullyDecoded);
    list << QUrl::fromLocalFile(tmp2.fileName()).toEncoded(QUrl::FullyDecoded);
    DRecentManager::removeItems(list);
    EXPECT_EQ(bookmarkCount(), 0);

    QFile::remove(tmp2.fileName());
}

TEST_F(ut_DRecentManager, removeItemsNoFile)
{
    QFile::remove(RecentPath);
    QStringList list;
    list << "/tmp/some_file.txt";
    DRecentManager::removeItems(list);
}

TEST_F(ut_DRecentManager, removeItemsInvalidFile)
{
    {
        QFile file(RecentPath);
        file.open(QIODevice::WriteOnly);
        file.write("not valid xml");
        file.close();
    }
    QStringList list;
    list << "/tmp/some_file.txt";
    DRecentManager::removeItems(list);
}

TEST_F(ut_DRecentManager, addItemCreatesNewXbelIfNotExists)
{
    QFile::remove(RecentPath);
    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "text/plain";

    EXPECT_TRUE(DRecentManager::addItem(m_testFile, data));
    EXPECT_TRUE(QFile::exists(RecentPath));

    QFile file(RecentPath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(&file));
    file.close();
    EXPECT_EQ(doc.documentElement().tagName(), "xbel");
}

TEST_F(ut_DRecentManager, addItemWithDirectory)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    DRecentData data;
    data.appExec = "test-exec";
    data.appName = "test-app";
    data.mimeType = "";
    EXPECT_TRUE(DRecentManager::addItem(tmpDir.path(), data));
    EXPECT_TRUE(bookmarkExists(tmpDir.path()));
}
