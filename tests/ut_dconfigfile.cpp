/*
 * Copyright (C) 2021 Uniontech Technology Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <DConfigFile>
#include <DStandardPaths>
#include <QBuffer>
#include <QDir>

#include <gtest/gtest.h>
#include "test_helper.hpp"

DCORE_USE_NAMESPACE

class ut_DConfigFile : public testing::Test
{
protected:
    static void SetUpTestCase() {
        dsgDataDir.set("DSG_DATA_DIR", "/tmp/dconfig/dsg_data");
        home.set("HOME", "/tmp/home");
    }
    static void TearDownTestCase() {
        dsgDataDir.restore();
        home.restore();
    }
    virtual void TearDown() override;

    static constexpr char const *LocalPrefix = "/tmp/example";

    const char *APP_ID = "org.foo.appid";
    const char *FILE_NAME = "org.foo.name";
    QString metaPath = QString("%1/opt/apps/%2/files/schemas/configs").arg(LocalPrefix, APP_ID);
    QString metaGlobalPath = QString("%1%2/configs").arg(LocalPrefix, DStandardPaths::path(DStandardPaths::DSG::DataDir));
    QString overridePath = QString("%1%2/configs/overrides/%3/%4").arg(LocalPrefix, DStandardPaths::path(DStandardPaths::DSG::DataDir), APP_ID, FILE_NAME);
    uint uid = getuid();
    static EnvGuard dsgDataDir;
    static EnvGuard home;
};
EnvGuard ut_DConfigFile::dsgDataDir;
EnvGuard ut_DConfigFile::home;


void ut_DConfigFile::TearDown() {
    QDir(LocalPrefix).removeRecursively();
}

TEST_F(ut_DConfigFile, testLoad) {
    QByteArray meta = R"delimiter(
{
    "magic": "dsg.config.meta",
    "version": "1.0",
    "contents": {
        "canExit": {
            "value": false,
            "serial": 0,
            "name": "I am name",
            "name[zh_CN]": "我是名字",
            "description": "我是描述",
            "description[en_US]": "I am description",
            "permissions": "readwrite",
            "visibility": "private"
        }
    }
}
        )delimiter";

    QBuffer buffer;
    buffer.setData(meta);

    DConfigFile config(APP_ID, FILE_NAME);
    ASSERT_TRUE(config.load(&buffer, {}));
    ASSERT_EQ(config.metaService()->keyList(), QStringList{QLatin1String("canExit")});

    ASSERT_EQ(config.value("canExit", uid).toBool(), false);

    ASSERT_EQ(config.metaService()->version().major, 1);
    ASSERT_EQ(config.metaService()->version().minor, 0);

    ASSERT_EQ(config.metaService()->visibility("canExit"), DConfigFile::Private);

    ASSERT_EQ(config.metaService()->displayName("canExit", QLocale::AnyLanguage), "I am name");
    ASSERT_EQ(config.metaService()->displayName("canExit", QLocale::Chinese), QString("我是名字"));

    ASSERT_EQ(config.metaService()->description("canExit", QLocale::AnyLanguage), "我是描述");
    ASSERT_EQ(config.metaService()->description("canExit", QLocale::English), "I am description");

    ASSERT_EQ(config.metaService()->permissions("canExit"), DConfigFile::ReadWrite);
}

TEST_F(ut_DConfigFile, fileIODevice) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.addCacheService(uid);
        config.load(LocalPrefix);

        config.setValue("canExit", false, uid, "appid");
        config.setValue("key2", QString("128"), uid);

        ASSERT_TRUE(config.save(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.addCacheService(uid);
        ASSERT_TRUE(config.load(LocalPrefix));
        ASSERT_EQ(config.value("canExit", uid), false);
        ASSERT_EQ(config.value("key2", uid).toString(), QString("128"));
    }
}

TEST_F(ut_DConfigFile, appmeta) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);

        config.load(LocalPrefix);
        ASSERT_EQ(config.value("key3", uid).toString(), QString("application"));
    }
}

TEST_F(ut_DConfigFile, globalmeta) {

    FileCopyGuard gurad(":/data/dconf-global.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    DConfigFile config(APP_ID, FILE_NAME);
    ASSERT_TRUE(config.load(LocalPrefix));
    ASSERT_EQ(config.value("key3", uid), QString("global"));
}

TEST_F(ut_DConfigFile, meta) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    FileCopyGuard gurad2(":/data/dconf-global.meta.json", QString("%1/%2.json").arg(metaGlobalPath, FILE_NAME));
    DConfigFile config(APP_ID, FILE_NAME);
    ASSERT_TRUE(config.load(LocalPrefix));
    ASSERT_EQ(config.value("key3", uid), QString("application"));
}

TEST_F(ut_DConfigFile, fileOverride) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        ASSERT_EQ(config.value("key3", uid), QString("application"));
    }

    FileCopyGuard gurad1(":/data/dconf-example.override.json", QString("%1/%2.json").arg(overridePath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        ASSERT_EQ(config.value("key3", uid), QString("override"));
    }

    FileCopyGuard gurad2(":/data/dconf-override/dconf-example.override.a.json", QString("%1/a/%2.json").arg(overridePath, FILE_NAME));
    {
        {
            DConfigFile config(APP_ID, FILE_NAME, "/a");
            config.load(LocalPrefix);
            ASSERT_EQ(config.value("key3", uid).toString(), QString("override /a"));
        }
    }

    FileCopyGuard gurad3(":/data/dconf-override/dconf-example.override.a.b.json", QString("%1/a/b/%2.json").arg(overridePath, FILE_NAME));
    {
        {
            DConfigFile config(APP_ID, FILE_NAME, "/a/b");
            config.load(LocalPrefix);
            ASSERT_EQ(config.value("key3", uid).toString(), QString("override /a/b"));
        }
    }
}
