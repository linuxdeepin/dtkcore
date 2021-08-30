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
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

class ut_DCI : public testing::Test {
public:
    void SetUp() override {
        QLoggingCategory::setFilterRules("dtk.dci.file.debug=true\n"
                                         "dtk.dci.fileengine.debug=true");
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

    // DDciFile::copy
    {
        DDciFile dciFile;
        // 文件复制
        ASSERT_TRUE(dciFile.writeFile("/test.txt", "test"));
        ASSERT_TRUE(dciFile.copy("/test.txt", "/test.txt.new"));
        ASSERT_TRUE(dciFile.exists("/test.txt.new"));
        ASSERT_EQ(dciFile.list("/", true), (QStringList{"test.txt", "test.txt.new"}));
        ASSERT_EQ(dciFile.dataRef("/test.txt"), dciFile.dataRef("/test.txt.new"));

        // 目录复制
        ASSERT_TRUE(dciFile.mkdir("/test"));
        ASSERT_TRUE(dciFile.rename("/test.txt", "/test/test.txt"));
        ASSERT_TRUE(dciFile.copy("/test", "/test.new"));
        ASSERT_TRUE(dciFile.exists("/test.new/test.txt"));
        ASSERT_EQ(dciFile.list("/test.new", true), (QStringList{"test.txt"}));
        ASSERT_EQ(dciFile.dataRef("/test/test.txt"), dciFile.dataRef("/test.new/test.txt"));
    }
}

class TestDCIFileHelper {
public:
    TestDCIFileHelper(const QString &fileName)
        : fileName(fileName)
    {
        if (QFile::exists(fileName))
            QFile::remove(fileName);
    }
    ~TestDCIFileHelper() {
        QFile::remove(fileName);
    }

    inline QString dciFormatFilePath(const QString &subfile = QString()) const {
        return "dci:" + fileName + subfile;
    }

    inline QString sourceFileName() const {
        return fileName;
    }

private:
    QString fileName;
};

TEST_F(ut_DCI, DFileEngine) {
    {
        TestDCIFileHelper helper(QDir::temp().absoluteFilePath("test.dci"));
        // 空 dci 文件创建
        QFile file(helper.dciFormatFilePath());
        ASSERT_TRUE(file.exists());
        QFileInfo info(file);
        ASSERT_TRUE(info.isDir());
        // 文件夹不可写入
        ASSERT_FALSE(file.open(QIODevice::WriteOnly));
        // 空文件遍历
        QDir dir(info.absoluteFilePath());
        ASSERT_EQ(dir.entryList(), QStringList{});
    }

    {
        TestDCIFileHelper helper(QDir::temp().absoluteFilePath("test.dci"));
        {
            // 内部文件创建
            QFile file(helper.dciFormatFilePath("/test.txt"));
            ASSERT_FALSE(file.open(QIODevice::ReadOnly));
            ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::WriteOnly));
            ASSERT_TRUE(file.exists());
            ASSERT_TRUE(QFileInfo(file).isFile());
            ASSERT_TRUE(file.write("Hello") == 5);
            ASSERT_TRUE(file.seek(0));
            ASSERT_EQ(file.readAll(), "Hello");
            ASSERT_TRUE(file.flush());
            file.close();

            // 文件信息
            QFileInfo info1(file);
            QFileInfo info2(helper.sourceFileName());
            ASSERT_EQ(info1.permissions(), info2.permissions());
            ASSERT_EQ(info1.fileTime(QFile::FileAccessTime),
                      info2.fileTime(QFile::FileAccessTime));
            ASSERT_EQ(info1.fileTime(QFile::FileBirthTime),
                      info2.fileTime(QFile::FileBirthTime));
            ASSERT_EQ(info1.fileTime(QFile::FileMetadataChangeTime),
                      info2.fileTime(QFile::FileMetadataChangeTime));
            ASSERT_EQ(info1.fileTime(QFile::FileModificationTime),
                      info2.fileTime(QFile::FileModificationTime));
            ASSERT_EQ(info1.ownerId(), info2.ownerId());
            ASSERT_EQ(info1.owner(), info2.owner());
            ASSERT_EQ(info1.groupId(), info2.groupId());
            ASSERT_EQ(info1.group(), info2.group());

            // 目录遍历
            QDir dir(helper.dciFormatFilePath());
            ASSERT_EQ(dir.entryList(), QStringList{"test.txt"});

            // 文件大小
            ASSERT_EQ(file.size(), 5);
            ASSERT_TRUE(file.resize(10));
            ASSERT_EQ(file.size(), 10);
        }

        {
            // 文件读取
            QFile file(helper.dciFormatFilePath("/test.txt"));
            ASSERT_TRUE(file.exists());
            ASSERT_TRUE(file.open(QIODevice::ReadOnly));
            ASSERT_EQ(file.readAll(), QByteArrayLiteral("Hello\0\0\0\0\0"));
            file.close();
            // [/test.txt]
        }

        {
            // 文件内容改写
            QFile file(helper.dciFormatFilePath("/test.txt"));
            ASSERT_TRUE(file.open(QIODevice::ReadWrite));
            ASSERT_TRUE(file.seek(1));
            ASSERT_TRUE(file.putChar('E'));
            char ch;
            ASSERT_TRUE(file.getChar(&ch));
            ASSERT_EQ(ch, 'l');
            ASSERT_EQ(file.readAll(), QByteArrayLiteral("lo\0\0\0\0\0"));
            ASSERT_TRUE(file.seek(0));
            ASSERT_EQ(file.readAll(), QByteArrayLiteral("HEllo\0\0\0\0\0"));
        }

        // 目录创建
        ASSERT_TRUE(QDir(helper.dciFormatFilePath()).mkdir("1"));
        ASSERT_FALSE(QDir(helper.dciFormatFilePath()).mkdir("2/3"));
        ASSERT_TRUE(QDir(helper.dciFormatFilePath()).mkpath("2/3"));
        ASSERT_TRUE(QFileInfo(helper.dciFormatFilePath("/1")).isDir());
        // [/test.txt, /1, /2, /2/3]

        // 目录 rename
        ASSERT_FALSE(QFile::rename(helper.dciFormatFilePath("/1"), "/1"));
        ASSERT_TRUE(QFile::rename(helper.dciFormatFilePath("/1"),
                                  helper.dciFormatFilePath("/1.new")));
        ASSERT_TRUE(QFile::rename(helper.dciFormatFilePath("/2"),
                                  helper.dciFormatFilePath("/2.new")));
        ASSERT_TRUE(QFile::rename(helper.dciFormatFilePath("/2.new/3"),
                                  helper.dciFormatFilePath("/3")));
        // [/test.txt, /1.new, /2.new, /3]

        // 文件 rename
        ASSERT_TRUE(QFile::rename(helper.dciFormatFilePath("/test.txt"),
                                  helper.dciFormatFilePath("/test.txt.new")));
        ASSERT_TRUE(QFile::rename(helper.dciFormatFilePath("/test.txt.new"),
                                  helper.dciFormatFilePath("/1.new/test.txt")));
        // [/1.new, /1.new/test.txt.new, /2.new, /3]

        // 复制
        ASSERT_TRUE(QFile::copy(helper.dciFormatFilePath("/1.new/test.txt"),
                                helper.dciFormatFilePath("/test.txt")));
        // 复制目录
        ASSERT_TRUE(QFile::copy(helper.dciFormatFilePath("/1.new"),
                                 helper.dciFormatFilePath("/1")));
        ASSERT_EQ(QDir(helper.dciFormatFilePath("/1")).entryList(),
                  QDir(helper.dciFormatFilePath("/1.new")).entryList());
        // [/1.new, /1.new/test.txt, /2.new, /3, /test.txt, "/1"]

        // 目录遍历
        QStringList list {
            helper.dciFormatFilePath("/1"),
            helper.dciFormatFilePath("/1/test.txt"),
            helper.dciFormatFilePath("/1.new"),
            helper.dciFormatFilePath("/1.new/test.txt"),
            helper.dciFormatFilePath("/2.new"),
            helper.dciFormatFilePath("/3"),
            helper.dciFormatFilePath("/test.txt")
        };
        QDirIterator di(helper.dciFormatFilePath(), QDirIterator::Subdirectories);
        while (di.hasNext()) {
            const QString &file = di.next();
            ASSERT_TRUE(list.removeOne(file));
        }
        ASSERT_TRUE(list.isEmpty());

        // 删除
        ASSERT_TRUE(QFile::remove(helper.dciFormatFilePath("/test.txt")));
        ASSERT_TRUE(QFile::remove(helper.dciFormatFilePath("/2.new")));
        ASSERT_TRUE(QFile::remove(helper.dciFormatFilePath("/1.new")));
        // [/3]
        ASSERT_EQ(QDir(helper.dciFormatFilePath()).entryList(),
                  (QStringList {"1", "3"}));
    }
}
