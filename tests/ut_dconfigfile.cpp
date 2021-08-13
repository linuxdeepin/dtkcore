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
#include <QBuffer>

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

class ut_DConfigFile : public testing::Test
{
protected:
    static void SetUpTestCase() {

    }
    static void TearDownTestCase() {

    }
    virtual void SetUp() override {

    }
    virtual void TearDown() override {

    }
};

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
            "description": "I am description",
            "permissions": "readwrite",
            "visibility": "private"
        }
    }
}
        )delimiter";

    QBuffer buffer;
    buffer.setData(meta);

    DConfigFile config("org.foo.appid", "org.foo.name");
    ASSERT_TRUE(config.load(&buffer, nullptr, nullptr, {}));
    ASSERT_EQ(config.keyList(), QStringList{QLatin1String("canExit")});

    config.setValue("canExit", true, "user", "appid");
    ASSERT_EQ(config.value("canExit").toBool(), true);
}
