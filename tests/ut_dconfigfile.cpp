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

static constexpr char const *LocalPrefix = "/tmp/example";

class ut_DConfigFile : public testing::Test
{
protected:
    static void SetUpTestCase() {
        home.set("HOME", "/tmp/home");
        dsgDataDir.set("DSG_DATA_DIRS", PREFIX"/share/dsg");
    }
    static void TearDownTestCase() {
        dsgDataDir.restore();
        home.restore();
    }
    virtual void TearDown() override;

    const char *APP_ID = "org.foo.appid";
    const char *FILE_NAME = "org.foo.name";
    QString metaPath = QString("%1" PREFIX"/share/dsg/configs/%2").arg(LocalPrefix, APP_ID);
    QString metaGlobalPath = QString("%1" PREFIX"/share/dsg/configs").arg(LocalPrefix);
    QString overridePath = QString("%1" PREFIX"/share/dsg/configs/overrides/%2/%3").arg(LocalPrefix, APP_ID, FILE_NAME);
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
    ASSERT_EQ(config.meta()->keyList(), QStringList{QLatin1String("canExit")});

    QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
    ASSERT_EQ(config.value("canExit", userCache.get()).toBool(), false);

    ASSERT_EQ(config.meta()->version().major, 1);
    ASSERT_EQ(config.meta()->version().minor, 0);

    ASSERT_EQ(config.meta()->visibility("canExit"), DConfigFile::Private);

    ASSERT_EQ(config.meta()->displayName("canExit", QLocale::AnyLanguage), "I am name");
    ASSERT_EQ(config.meta()->displayName("canExit", QLocale::Chinese), QString("我是名字"));

    ASSERT_EQ(config.meta()->description("canExit", QLocale::AnyLanguage), "我是描述");
    ASSERT_EQ(config.meta()->description("canExit", QLocale::English), "I am description");

    ASSERT_EQ(config.meta()->permissions("canExit"), DConfigFile::ReadWrite);
}

TEST_F(ut_DConfigFile, setValueTypeCheck) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    DConfigFile config(APP_ID, FILE_NAME);
    config.load(LocalPrefix);
    QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
    userCache->load(LocalPrefix);
    {
        const auto type = config.value("canExit", userCache.get()).type();
        ASSERT_TRUE(config.setValue("canExit", false, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("canExit", "true", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("canExit", "true2", "test", userCache.get()));
        ASSERT_EQ(config.value("canExit", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("key2", userCache.get()).type();
        ASSERT_TRUE(config.setValue("key2", "121", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("key2", 121, "test", userCache.get()));
        ASSERT_EQ(config.value("key2", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("number", userCache.get()).type();
        ASSERT_TRUE(config.setValue("number", 1, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("number", 2.0, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("number", "3", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("number", "1ab", "test", userCache.get()));
        ASSERT_EQ(config.value("number", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("array", userCache.get()).type();
        const QStringList array{"value1", "value2"};
        ASSERT_TRUE(config.setValue("array", QStringList(), "test", userCache.get()));
        ASSERT_TRUE(config.setValue("array", array, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("array", QJsonDocument::fromJson("[]").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("array", "", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("array", "value1", "test", userCache.get()));
        ASSERT_EQ(config.value("array", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("array_map", userCache.get()).type();
        QVariantList array;
        QVariantMap map1;
        map1["key1"] = "value1";
        map1["key2"] = "value2";
        array.append(map1);
        ASSERT_EQ(config.value("array_map", userCache.get()).toList(), array);
        ASSERT_TRUE(config.setValue("array_map", QVariantList(), "test", userCache.get()));
        ASSERT_TRUE(config.setValue("array_map", array, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("array_map", QJsonDocument::fromJson("[]").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("array_map", "", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("array_map", "value1", "test", userCache.get()));
        ASSERT_EQ(config.value("array_map", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("map", userCache.get()).type();
        QVariantMap map;
        map.insert("key1", "value1");
        map.insert("key2", "value2");
        ASSERT_TRUE(config.setValue("map", QVariantMap(), "test", userCache.get()));
        ASSERT_TRUE(config.setValue("map", map, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("map", QJsonDocument::fromJson("{}").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("map", QJsonDocument::fromJson("[]").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("map", "key1", "test", userCache.get()));
        ASSERT_EQ(config.value("map", userCache.get()).type(), type);
    }
    {
        const auto type = config.value("map_array", userCache.get()).type();
        QVariantMap map;
        map.insert("key1", QStringList{"value1"});
        map.insert("key2", QStringList{"value2"});
        ASSERT_EQ(config.value("map_array", userCache.get()).toMap(), map);
        ASSERT_TRUE(config.setValue("map_array", QVariantMap(), "test", userCache.get()));
        ASSERT_TRUE(config.setValue("map_array", map, "test", userCache.get()));
        ASSERT_TRUE(config.setValue("map_array", QJsonDocument::fromJson("{}").toVariant(), "test", userCache.get()));
        ASSERT_FALSE(config.setValue("map_array", "", "test", userCache.get()));
        ASSERT_FALSE(config.setValue("map_array", "value1", "test", userCache.get()));
        ASSERT_EQ(config.value("map_array", userCache.get()).type(), type);
    }
}

TEST_F(ut_DConfigFile, fileIODevice) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        userCache->load(LocalPrefix);

        config.setValue("canExit", false, "test", userCache.get());
        config.setValue("key2", QString("128"), "test", userCache.get());

        ASSERT_TRUE(config.save(LocalPrefix));
        ASSERT_TRUE(userCache->save(LocalPrefix));
    }
    {
        DConfigFile config(APP_ID, FILE_NAME);
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("canExit", userCache.get()), false);
        ASSERT_EQ(config.value("key2", userCache.get()).toString(), QString("128"));
    }
}

TEST_F(ut_DConfigFile, appmeta) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);

        config.load(LocalPrefix);
        ASSERT_TRUE(config.load(LocalPrefix));
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("key3", userCache.get()).toString(), QString("application"));
    }
}

TEST_F(ut_DConfigFile, globalmeta) {

    FileCopyGuard gurad(":/data/dconf-global.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    DConfigFile config(APP_ID, FILE_NAME);
    ASSERT_TRUE(config.load(LocalPrefix));
    QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
    ASSERT_TRUE(userCache->load(LocalPrefix));
    ASSERT_EQ(config.value("key3", userCache.get()), QString("global"));
}

TEST_F(ut_DConfigFile, meta) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    FileCopyGuard gurad2(":/data/dconf-global.meta.json", QString("%1/%2.json").arg(metaGlobalPath, FILE_NAME));
    DConfigFile config(APP_ID, FILE_NAME);
    ASSERT_TRUE(config.load(LocalPrefix));
    QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
    ASSERT_TRUE(userCache->load(LocalPrefix));
    ASSERT_EQ(config.value("key3", userCache.get()), QString("application"));
    const QStringList array{"value1", "value2"};
    ASSERT_EQ(config.value("array", userCache.get()), array);
    QVariantMap map;
    map.insert("key1", "value1");
    map.insert("key2", "value2");
    ASSERT_EQ(config.value("map", userCache.get()).toMap(), map);
}

TEST_F(ut_DConfigFile, fileOverride) {

    FileCopyGuard gurad(":/data/dconf-example.meta.json", QString("%1/%2.json").arg(metaPath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("key3", userCache.get()), QString("application"));
    }

    FileCopyGuard gurad1(":/data/dconf-example.override.json", QString("%1/%2.json").arg(overridePath, FILE_NAME));
    {
        DConfigFile config(APP_ID, FILE_NAME);
        config.load(LocalPrefix);
        QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
        ASSERT_TRUE(userCache->load(LocalPrefix));
        ASSERT_EQ(config.value("key3", userCache.get()), QString("override"));
    }

    FileCopyGuard gurad2(":/data/dconf-override/dconf-example.override.a.json", QString("%1/a/%2.json").arg(overridePath, FILE_NAME));
    {
        {
            DConfigFile config(APP_ID, FILE_NAME, "/a");
            config.load(LocalPrefix);
            QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
            ASSERT_TRUE(userCache->load(LocalPrefix));
            ASSERT_EQ(config.value("key3", userCache.get()).toString(), QString("override /a"));
        }
    }

    FileCopyGuard gurad3(":/data/dconf-override/dconf-example.override.a.b.json", QString("%1/a/b/%2.json").arg(overridePath, FILE_NAME));
    {
        {
            DConfigFile config(APP_ID, FILE_NAME, "/a/b");
            config.load(LocalPrefix);
            QScopedPointer<DConfigCache> userCache(config.createUserCache(uid));
            ASSERT_TRUE(userCache->load(LocalPrefix));
            ASSERT_EQ(config.value("key3", userCache.get()).toString(), QString("override /a/b"));
        }
    }
}
