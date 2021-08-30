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
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include "settings/dsettings.h"
#include "settings/dsettingsoption.h"
#include "settings/dsettingsgroup.h"
#include "settings/backend/gsettingsbackend.h"
#include "settings/backend/qsettingbackend.h"

DCORE_USE_NAMESPACE


class ut_GSettings : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    DSettings *settings = nullptr;
    GSettingsBackend * gSettingBackend = nullptr;
    QString jsonContent;
};

void ut_GSettings::SetUp()
{
    QFile file("/tmp/test.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    jsonContent = " {     "
                  " \"gsettings\": { "
                  " \"id\": \"com.deepin.dtk\","
                  " \"path\": \"/dtk/deepin/deepin-terminal/\" "
                  "},"
                  " \"groups\": [{ "
                  " \"key\": \"base\", "
                  " \"name\": \"Basic settings\", "
                  " \"groups\": [{ "
                  " \"key\": \"open_action\", "
                  " \"name\": \"Open Action\", "
                  " \"options\": [{ "
                  " \"key\": \"paletteType\", "
                  " \"type\": \"checkbox\", "
                  " \"text\": \"Always Open On New Windows\", "
                  " \"default\": true "
                  " }]  "
                  " }] }]"
                  "}";
    out << jsonContent;
    file.close();
    settings = DSettings::fromJson(jsonContent.toLatin1());
    gSettingBackend = new GSettingsBackend(settings);

}

void ut_GSettings::TearDown()
{
    if (gSettingBackend) {
        delete gSettingBackend;
        gSettingBackend = nullptr;
    }
    if (settings) {
        delete settings;
        settings = nullptr;
    }
    QFile file("/tmp/test.json");
    if (file.exists())
        file.remove();
}

TEST_F(ut_GSettings, testGSettingBackendKeys)
{
    QStringList qKeys = gSettingBackend->keys();
    ASSERT_TRUE(!qKeys.isEmpty());
}

TEST_F(ut_GSettings, testGSettingBackendGetOption)
{
    QStringList qKeys = gSettingBackend->keys();
    ASSERT_TRUE(!qKeys.isEmpty());
    QVariant value = gSettingBackend->getOption(qKeys[0]);
    ASSERT_TRUE(!value.toString().isEmpty());
}


