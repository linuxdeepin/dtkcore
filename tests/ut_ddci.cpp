/*
 * Copyright (C) 2021 UnionTech Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@uniontech.com>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <DDciFile>
#include <QLoggingCategory>

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

class ut_DCI : public testing::Test {
public:
    void SetUp() override {
        QLoggingCategory::setFilterRules("dtk.dci.file.debug=true");
    }
};

TEST_F(ut_DCI, DDciFile) {
    {
        // 空文件
        DDciFile dciFile;
        ASSERT_TRUE(dciFile.isValid());
        ASSERT_TRUE(dciFile.exists("/"));
        ASSERT_EQ(dciFile.list("/").count(), 0);
        ASSERT_EQ(dciFile.toData(), QByteArrayLiteral("DCI\0\1\0\0\0"));
    }

    {
        // 最小数据文件
        DDciFile dciFile(QByteArrayLiteral("DCI\0\1\0\0\0"));
        ASSERT_TRUE(dciFile.isValid());
        ASSERT_TRUE(dciFile.exists("/"));
        ASSERT_EQ(dciFile.list("/").count(), 0);
        ASSERT_EQ(dciFile.toData(), QByteArrayLiteral("DCI\0\1\0\0\0"));
    }

    {
        // 错误的版本
        DDciFile dciFile(QByteArrayLiteral("DCI\0\0\0\0\0"));
        ASSERT_FALSE(dciFile.isValid());
    }

    {
        // 错误的文件数量
        DDciFile dciFile(QByteArrayLiteral("DCI\0\1\255\255\255"));
        ASSERT_FALSE(dciFile.isValid());
    }

    {
        // 创建目录
        DDciFile dciFile;
        ASSERT_TRUE(dciFile.mkdir("/test"));
        ASSERT_TRUE(dciFile.exists("/test"));
        ASSERT_EQ(dciFile.list("/"), QStringList{"/test"});
        ASSERT_EQ(dciFile.childrenCount("/"), 1);
    }

    {
        // 创建目录
        DDciFile dciFile;
        ASSERT_TRUE(dciFile.mkdir("/test"));
        ASSERT_TRUE(dciFile.exists("/test"));
        ASSERT_EQ(dciFile.list("/"), QStringList{"/test"});
        ASSERT_EQ(dciFile.type("/test"), DDciFile::Directory);
        ASSERT_EQ(dciFile.childrenCount("/"), 1);

        // 创建文件
        ASSERT_TRUE(dciFile.writeFile("/test/test.txt", "test\n"));
        ASSERT_TRUE(dciFile.exists("/test/test.txt"));
        ASSERT_EQ(dciFile.list("/test"), QStringList{"/test/test.txt"});
        ASSERT_EQ(dciFile.childrenCount("/test"), 1);
        ASSERT_EQ(dciFile.type("/test/test.txt"), DDciFile::File);
        ASSERT_EQ(dciFile.dataRef("/test/test.txt"), QByteArray("test\n"));

        {
            // 复制数据
            DDciFile dciFile2(dciFile.toData());
            ASSERT_TRUE(dciFile2.isValid());
            ASSERT_EQ(dciFile2.list("/"), QStringList{"/test"});
            ASSERT_EQ(dciFile2.list("/test"), QStringList{"/test/test.txt"});
            ASSERT_EQ(dciFile2.dataRef("/test/test.txt"), QByteArray("test\n"));
            ASSERT_EQ(dciFile.toData(), dciFile2.toData());
        }

        // 改写文件数据
        ASSERT_FALSE(dciFile.writeFile("/test/test.txt", "override the\"test.txt\""));
        ASSERT_TRUE(dciFile.writeFile("/test/test.txt", "override the\"test.txt\"", true));
        ASSERT_EQ(dciFile.dataRef("/test/test.txt"), "override the\"test.txt\"");

        // 文件删除
        ASSERT_TRUE(dciFile.remove("/test/test.txt"));
        ASSERT_FALSE(dciFile.exists("/test/test.txt"));
        ASSERT_EQ(dciFile.list("/test"), QStringList{});

        // 目录删除
        ASSERT_TRUE(dciFile.writeFile("/test/test.txt", ""));
        ASSERT_TRUE(dciFile.remove("/test"));
        ASSERT_FALSE(dciFile.exists("/test"));
        ASSERT_EQ(dciFile.list("/"), QStringList{});
        ASSERT_EQ(dciFile.toData(), QByteArrayLiteral("DCI\0\1\0\0\0"));
    }

    {
        DDciFile dciFile;
        ASSERT_TRUE(dciFile.mkdir("/test"));
        ASSERT_TRUE(dciFile.writeFile("/test.txt", ""));

        // 重命名
        ASSERT_TRUE(dciFile.rename("/test.txt", "/test/new_test.txt"));
        ASSERT_TRUE(dciFile.exists("/test/new_test.txt"));
        ASSERT_FALSE(dciFile.rename("/test.txt", "/test/new_test.txt"));
        ASSERT_EQ(dciFile.list("/test", true), QStringList{"new_test.txt"});
        ASSERT_EQ(dciFile.list("/test"), QStringList{"/test/new_test.txt"});
        // override 重命名
        ASSERT_TRUE(dciFile.writeFile("/test.txt", "test", false));
        ASSERT_FALSE(dciFile.rename("/test.txt", "/test/new_test.txt", false));
        ASSERT_TRUE(dciFile.rename("/test.txt", "/test/new_test.txt", true));
        ASSERT_EQ(dciFile.dataRef("/test/new_test.txt"), "test");

        // 文件清空
        ASSERT_TRUE(dciFile.remove("/"));
        ASSERT_EQ(dciFile.list("/"), QStringList{});
        ASSERT_EQ(dciFile.childrenCount("/"), 0);
        ASSERT_EQ(dciFile.toData(), QByteArrayLiteral("DCI\0\1\0\0\0"));
    }
}
