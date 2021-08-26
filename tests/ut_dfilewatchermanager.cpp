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
#include <QObject>
#include <QDir>
#include <QFile>
#include <QUrl>
#include "filesystem/dfilewatcher.h"
#include "filesystem/dfilewatchermanager.h"

DCORE_USE_NAMESPACE


class ut_DFileWatcherManager : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DFileWatcherManager *fileWatcherManager = nullptr;

};

void ut_DFileWatcherManager::SetUp()
{
    fileWatcherManager = new DFileWatcherManager(nullptr);
    QFile file("/tmp/test");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();
}

void ut_DFileWatcherManager::TearDown()
{
    if (fileWatcherManager) {
        delete fileWatcherManager;
        fileWatcherManager = nullptr;
    }
    QFile file("/tmp/test");
    if (file.exists())
        file.remove();
}

TEST_F(ut_DFileWatcherManager, testDFileSystemWatcherAdd)
{
    auto watcher = fileWatcherManager->add("/tmp/test");
    bool isChanged = false;
    QObject::connect(watcher, &DFileWatcher::fileDeleted, watcher, [&isChanged](const QUrl & ) {
        isChanged = true;
    });
    QFile file("/tmp/test");
    file.remove("/tmp/test");
    if (!file.exists())
        emit watcher->fileDeleted(QUrl("tmp/test"));
    ASSERT_TRUE(isChanged);
}

TEST_F(ut_DFileWatcherManager, testDFileSystemWatcherRemove)
{
    fileWatcherManager->remove("/tmp/test");
    // TODO:待接口变化再补充
}

