/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Wang Fei <wangfeia@uniontech.com>
 *
 * Maintainer: Wang Fei <wangfeia@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <QFile>
#include <QDomDocument>
#include <QUrl>
#include <QDir>

#include "util/drecentmanager.h"

DCORE_USE_NAMESPACE

class ut_DRecentManager: public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void ut_DRecentManager::SetUp()
{
    QFile file("/tmp/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();
}

void ut_DRecentManager::TearDown()
{
    QFile file("/tmp/test");
    if (file.exists())
        file.remove();
}

TEST_F(ut_DRecentManager, testDRecentManagerAddItem)
{
    DRecentData data;
    data.appExec = "deepin-editor";
    data.appName = "Deepin Editor";
    data.mimeType = "text/plain";

    bool ok = DRecentManager::addItem("/tmp/test", data);
    bool isFound = false;
    QFile file(QDir::homePath() + "/.local/share/recently-used.xbel");
    QDomDocument doc;
    if (doc.setContent(&file)) {
        QDomElement rootEle = doc.documentElement();
        QDomNodeList nodeList = rootEle.elementsByTagName("bookmark");
        QDomElement bookmarkEle;
        QUrl url = QUrl::fromLocalFile("/tmp/test");
        for (int i = 0; i < nodeList.size(); ++i) {
            const QString fileUrl = nodeList.at(i).toElement().attribute("href");
            if (fileUrl == url.toEncoded(QUrl::FullyDecoded)) {
                bookmarkEle = nodeList.at(i).toElement();
                isFound = true;
                break;
            }
        }
        ASSERT_TRUE(isFound == ok);
    }
}

TEST_F(ut_DRecentManager, testDRecentManagerRemoveItem)
{
    QString testFile = QUrl::fromLocalFile("/tmp/test").toEncoded(QUrl::FullyDecoded);
    DRecentManager::removeItem(testFile);
    QFile file(QDir::homePath() + "/.local/share/recently-used.xbel");
    QDomDocument doc;
    bool isFound = false;
    if (doc.setContent(&file)) {
        QDomElement rootEle = doc.documentElement();
        QDomNodeList nodeList = rootEle.elementsByTagName("bookmark");
        QDomElement bookmarkEle;
        for (int i = 0; i < nodeList.size(); ++i) {
            const QString fileUrl = nodeList.at(i).toElement().attribute("href");
            if (fileUrl == testFile) {
                bookmarkEle = nodeList.at(i).toElement();
                isFound = true;
                break;
            }
        }
    }
    ASSERT_TRUE(!isFound);
}
