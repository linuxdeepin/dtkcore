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
#include <QDir>
#include "filesystem/dpathbuf.h"

DCORE_USE_NAMESPACE


class ut_DPathBuf : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DPathBuf *pathBuf = nullptr;

};

void ut_DPathBuf::SetUp()
{
    pathBuf = new DPathBuf("/tmp/etc");
    QDir dir("/tmp/etc/");
    if (!dir.exists())
        dir.mkdir("/tmp/etc/");
}

void ut_DPathBuf::TearDown()
{
    if (pathBuf) {
        delete pathBuf;
        pathBuf = nullptr;
    }
    QDir dir("/tmp/etc/");
    if (dir.exists())
        dir.remove("/tmp/etc/");
}

TEST_F(ut_DPathBuf, testDPathBufOperatorSlashQString)
{
    *pathBuf = *pathBuf / QString("test");
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testDPathBufOperatorSlashEqualQString)
{
    *pathBuf /= QString("test");
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testDPathBufOperatorSlashChar)
{
    *pathBuf = *pathBuf / "test";
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testDPathBufOperatorSlashEqualChar)
{
    *pathBuf /= "test";
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testDPathBufJoin)
{
    *pathBuf = pathBuf->join(QString("test"));
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc/test");
}

TEST_F(ut_DPathBuf, testToString)
{
    auto str = pathBuf->toString();
    ASSERT_TRUE(str == "/tmp/etc");
}
