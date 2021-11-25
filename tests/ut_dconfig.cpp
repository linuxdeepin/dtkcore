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

class ut_DConfig : public testing::Test
{
protected:
    static void SetUpTestCase() {
        fileBackendLocalPerfix.set("DSG_DCONFIG_FILE_BACKEND_LOCAL_PREFIX", "/tmp/example");
        metaGuard = new FileCopyGuard(":/data/dconf-example.meta.json", QString("%1/opt/apps/%2/files/schemas/configs/%3.json").arg(fileBackendLocalPerfix.value(), APP_ID, FILE_NAME));

        backendType.set("DSG_DCONFIG_BACKEND_TYPE", "FileBackend");
    }
    static void TearDownTestCase() {
        QDir(fileBackendLocalPerfix.value()).removeRecursively();
        fileBackendLocalPerfix.restore();
        delete metaGuard;

        backendType.restore();
    }
    virtual void SetUp() override;

    static EnvGuard backendType;
    static EnvGuard fileBackendLocalPerfix;
    static FileCopyGuard *metaGuard;
    static constexpr char const *APP_ID = "tests";
    static constexpr char const *FILE_NAME = "example";
};
EnvGuard ut_DConfig::fileBackendLocalPerfix;
EnvGuard ut_DConfig::backendType;
FileCopyGuard *ut_DConfig::metaGuard = nullptr;
constexpr char const *ut_DConfig::APP_ID;
constexpr char const *ut_DConfig::FILE_NAME;

TEST_F(ut_DConfig, backend) {

    DConfig config(FILE_NAME);
    ASSERT_EQ(config.backendName(), QString("FileBackend"));
}

TEST_F(ut_DConfig, isValid) {

    DConfig config(FILE_NAME);
    ASSERT_TRUE(config.isValid());
}

TEST_F(ut_DConfig, value) {
    {
        DConfig config(FILE_NAME);
        config.setValue("key2", "126");
        ASSERT_EQ(config.value("key2").toString(), QString("126"));
    }
    {
        DConfig config(FILE_NAME);
        ASSERT_EQ(config.value("key2").toString(), QString("126"));
    }
}

TEST_F(ut_DConfig, keyList) {

    DConfig config(FILE_NAME);
    QStringList keyList{QString("key2"), QString("canExit")};
    for (auto item : keyList) {
        ASSERT_TRUE(config.keyList().contains(item));
    }
}

void ut_DConfig::SetUp() {}
