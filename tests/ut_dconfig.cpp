/*
 * Copyright (C) 2021 Uniontech Technology Co., Ltd.
 *
 * Author:     yeshanshan <yeshanshan@live.com>
 *
 * Maintainer: yeshanshan <yeshanshan@uniontech.com>
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

#include <DConfig>
#include <QBuffer>
#include <QDir>
#include <QDebug>

#include <gtest/gtest.h>
#include "test_helper.hpp"

DCORE_USE_NAMESPACE

static constexpr char const *APP_ID = "tests";
static constexpr char const *FILE_NAME = "example";
static EnvGuard dsgDataDir;
class ut_DConfig : public testing::Test
{
protected:
    static void SetUpTestCase() {
        fileBackendLocalPerfix.set("DSG_DCONFIG_FILE_BACKEND_LOCAL_PREFIX", "/tmp/example");
        metaGuard = new FileCopyGuard(":/data/dconf-example.meta.json", QString("%1" PREFIX"/share/dsg/configs/%2/%3.json").arg(fileBackendLocalPerfix.value(), APP_ID, FILE_NAME));

        backendType.set("DSG_DCONFIG_BACKEND_TYPE", "FileBackend");
        dsgDataDir.set("DSG_DATA_DIRS", PREFIX"/share/dsg");
    }
    static void TearDownTestCase() {
        QDir(fileBackendLocalPerfix.value()).removeRecursively();
        fileBackendLocalPerfix.restore();
        delete metaGuard;

        backendType.restore();
        dsgDataDir.restore();
    }
    virtual void SetUp() override;

    static EnvGuard backendType;
    static EnvGuard fileBackendLocalPerfix;
    static FileCopyGuard *metaGuard;
};
EnvGuard ut_DConfig::fileBackendLocalPerfix;
EnvGuard ut_DConfig::backendType;
FileCopyGuard *ut_DConfig::metaGuard = nullptr;

TEST_F(ut_DConfig, backend) {

    DConfig config(FILE_NAME);
    ASSERT_EQ(config.backendName(), QString("FileBackend"));
}

TEST_F(ut_DConfig, isValid) {

    DConfig config(FILE_NAME);
    ASSERT_TRUE(config.isValid());
}

TEST_F(ut_DConfig, value) {
    const QStringList array{"1", "2"};
    QVariantMap map;
    map.insert("key1", "value1");
    map.insert("key2", "value2");
    {
        DConfig config(FILE_NAME);
        config.setValue("key2", "126");
        ASSERT_EQ(config.value("key2").toString(), QString("126"));

        config.setValue("array", array);
        ASSERT_EQ(config.value("array").toStringList(), array);

        config.setValue("map", map);
        ASSERT_EQ(config.value("map").toMap(), map);
    }
    {
        DConfig config(FILE_NAME);
        ASSERT_EQ(config.value("key2").toString(), QString("126"));
        ASSERT_EQ(config.value("array").toStringList(), array);
        ASSERT_EQ(config.value("map").toMap(), map);
    }
    {
        DConfig config(FILE_NAME);
        config.reset("canExit");
        ASSERT_EQ(config.value("canExit").toBool(), true);

        config.reset("key2");
        ASSERT_EQ(config.value("key2").toString(), QString("125"));

        config.reset("number");
        ASSERT_EQ(config.value("number").toInt(), 1);

        config.reset("array");
        const QStringList &originArray {"value1", "value2"};
        ASSERT_EQ(config.value("array").toStringList(), originArray);

        config.reset("map");
        QVariantMap originMap;
        originMap.insert("key1", "value1");
        originMap.insert("key2", "value2");
        ASSERT_EQ(config.value("map").toMap(), originMap);
    }
}

TEST_F(ut_DConfig, keyList) {

    DConfig config(FILE_NAME);
    QStringList keyList{QString("key2"), QString("canExit")};
    for (auto item : keyList) {
        ASSERT_TRUE(config.keyList().contains(item));
    }
}

TEST_F(ut_DConfig, OtherAppConfigfile) {

    constexpr char const *APP_OTHER = "tests_other";
    FileCopyGuard gurand(":/data/dconf-example_other_app_configure.meta.json", QString("%1" PREFIX"/share/dsg/configs/%2/%3.json").arg(fileBackendLocalPerfix.value(), APP_OTHER, FILE_NAME));

    QScopedPointer<DConfig> config(DConfig::create(APP_OTHER, FILE_NAME));
    ASSERT_TRUE(config->isValid());
    ASSERT_EQ(config->value("appPublic").toString(), QString("publicValue"));
}

void ut_DConfig::SetUp() {}
