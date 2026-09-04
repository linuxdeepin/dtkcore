// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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

TEST_F(ut_DPathBuf, testDefaultConstructor)
{
    DPathBuf emptyBuf;
    auto str = emptyBuf.toString();
    ASSERT_FALSE(str.isEmpty());
}

TEST_F(ut_DPathBuf, testEmptyPathConstructor)
{
    DPathBuf emptyPath("");
    auto str = emptyPath.toString();
    ASSERT_FALSE(str.isEmpty());
}

TEST_F(ut_DPathBuf, testMultipleSegments)
{
    DPathBuf path("/tmp");
    path = path / "a" / "b" / "c";
    auto str = path.toString();
    ASSERT_TRUE(str == "/tmp/a/b/c");
}

TEST_F(ut_DPathBuf, testMultipleSegmentsWithSlashEqual)
{
    DPathBuf path("/tmp");
    path /= "a";
    path /= "b";
    path /= "c";
    auto str = path.toString();
    ASSERT_TRUE(str == "/tmp/a/b/c");
}

TEST_F(ut_DPathBuf, testJoinMultiple)
{
    DPathBuf path("/tmp");
    path.join("a").join("b").join("c");
    auto str = path.toString();
    ASSERT_TRUE(str == "/tmp/a/b/c");
}

TEST_F(ut_DPathBuf, testMixedOperators)
{
    DPathBuf path("/tmp");
    path = path / "a";
    path /= "b";
    path = path.join("c");
    auto str = path.toString();
    ASSERT_TRUE(str == "/tmp/a/b/c");
}

TEST_F(ut_DPathBuf, testRelativePath)
{
    DPathBuf path("relative/path");
    auto str = path.toString();
    ASSERT_FALSE(str.isEmpty());
}

TEST_F(ut_DPathBuf, testOperatorSlashReturnsNewObject)
{
    DPathBuf original("/tmp");
    DPathBuf result = original / "sub";
    ASSERT_EQ(original.toString(), "/tmp");
    ASSERT_EQ(result.toString(), "/tmp/sub");
}

TEST_F(ut_DPathBuf, testChainedFromDefaultConstructor)
{
    DPathBuf path;
    path = path / "tmp" / "test";
    auto str = path.toString();
    ASSERT_FALSE(str.isEmpty());
}

TEST_F(ut_DPathBuf, testDotPathNormalization)
{
    DPathBuf path("/tmp/./test");
    auto str = path.toString();
    ASSERT_TRUE(str == "/tmp/test");
}

TEST_F(ut_DPathBuf, testDoubleDotPathNormalization)
{
    DPathBuf path("/tmp/a/../b");
    auto str = path.toString();
    ASSERT_TRUE(str == "/tmp/b");
}
