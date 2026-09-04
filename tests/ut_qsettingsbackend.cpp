// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryFile>
#include "settings/dsettings.h"
#include "settings/dsettingsoption.h"
#include "settings/dsettingsgroup.h"
#include "settings/backend/gsettingsbackend.h"
#include "settings/backend/qsettingbackend.h"

DCORE_USE_NAMESPACE


class ut_QSettingsBackend : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    QString jsonContent;
    QString iniContent;
};

void ut_QSettingsBackend::SetUp()
{
    QFile file("/tmp/test.ini");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    jsonContent = " { \"groups\": [{ "
                  " \"key\": \"base\", "
                  " \"name\": \"Basic settings\", "
                  " \"groups\": [{ "
                  " \"key\": \"open_action\", "
                  " \"name\": \"Open Action\", "
                  " \"options\": [{ "
                  " \"key\": \"alway_open_on_new\", "
                  " \"type\": \"checkbox\", "
                  " \"text\": \"Always Open On New Windows\", "
                  " \"default\": true "
                  " }]  "
                  " }] }]}";
    iniContent = "[Test] \n \
                  value=false  ";

    out << iniContent;
    file.close();
}

void ut_QSettingsBackend::TearDown()
{
    QFile file("/tmp/test.ini");
    if (file.exists())
        file.remove();
}

TEST_F(ut_QSettingsBackend, testQSettingsBackendKeys)
{
    DSettings tmpSetting;
    QSettingBackend qBackend("/tmp/test.ini");
    tmpSetting.setBackend(&qBackend);
    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(!qKeys.isEmpty());
}

TEST_F(ut_QSettingsBackend, testQSettingsBackendGetOption)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJson(jsonContent.toLatin1());
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QSettingBackend qBackend("/tmp/test.ini");
    scopeSettings->setBackend(&qBackend);
    scopeSettings->sync();
    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(!qKeys.isEmpty());
    QVariant value = qBackend.getOption("Test");
    ASSERT_TRUE(!value.toBool());
}

TEST_F(ut_QSettingsBackend, testQSettingsBackendDoOption)
{
    QPointer<DSettings> tmpSetting = DSettings::fromJson(jsonContent.toLatin1());
    QScopedPointer<DSettings> scopeSettings(tmpSetting.data());
    QSettingBackend qBackend("/tmp/test.ini");
    scopeSettings->setBackend(&qBackend);
    Q_EMIT qBackend.setOption("Test", true);

    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(!qKeys.isEmpty());
    QVariant value = qBackend.getOption("Test");
    ASSERT_TRUE(!value.toBool());

    // ensure `DSettings` is released before `SettingBackend` if `doSetOption` maybe execute.
    scopeSettings.reset();
}

TEST_F(ut_QSettingsBackend, testDoSync)
{
    QSettingBackend qBackend("/tmp/test.ini");
    qBackend.doSync();
    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(!qKeys.isEmpty());
}

TEST_F(ut_QSettingsBackend, testDoSetOption)
{
    QSettingBackend qBackend("/tmp/test.ini");
    qBackend.doSetOption("TestGroup", QVariant(true));
    QVariant value = qBackend.getOption("TestGroup");
    ASSERT_TRUE(value.toBool());
}

TEST_F(ut_QSettingsBackend, testDoSetOptionSameValue)
{
    QSettingBackend qBackend("/tmp/test.ini");
    qBackend.doSetOption("SameValueTest", QVariant(42));
    QVariant value1 = qBackend.getOption("SameValueTest");
    ASSERT_EQ(value1.toInt(), 42);
    qBackend.doSetOption("SameValueTest", QVariant(42));
    QVariant value2 = qBackend.getOption("SameValueTest");
    ASSERT_EQ(value2.toInt(), 42);
}

TEST_F(ut_QSettingsBackend, testDoSetOptionDifferentValue)
{
    QSettingBackend qBackend("/tmp/test.ini");
    qBackend.doSetOption("DiffValueTest", QVariant("hello"));
    ASSERT_EQ(qBackend.getOption("DiffValueTest").toString(), "hello");
    qBackend.doSetOption("DiffValueTest", QVariant("world"));
    ASSERT_EQ(qBackend.getOption("DiffValueTest").toString(), "world");
}

TEST_F(ut_QSettingsBackend, testGetOptionNonExistentKey)
{
    QSettingBackend qBackend("/tmp/test.ini");
    QVariant value = qBackend.getOption("NonExistentKey");
    ASSERT_FALSE(value.isValid());
}

TEST_F(ut_QSettingsBackend, testKeysWithEmptyFile)
{
    QString emptyFile = "/tmp/test_empty.ini";
    QFile::remove(emptyFile);
    {
        QSettings settings(emptyFile, QSettings::NativeFormat);
        settings.sync();
    }
    QSettingBackend qBackend(emptyFile);
    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(qKeys.isEmpty());
    QFile::remove(emptyFile);
}

TEST_F(ut_QSettingsBackend, testConstructorWithNewFile)
{
    QString newFile = "/tmp/test_new_backend.ini";
    QFile::remove(newFile);
    QSettingBackend qBackend(newFile);
    QStringList qKeys = qBackend.keys();
    ASSERT_TRUE(qKeys.isEmpty());
    qBackend.doSetOption("NewGroup", QVariant("test_value"));
    ASSERT_EQ(qBackend.getOption("NewGroup").toString(), "test_value");
    QFile::remove(newFile);
}

TEST_F(ut_QSettingsBackend, testDoSetOptionMultipleKeys)
{
    QSettingBackend qBackend("/tmp/test.ini");
    qBackend.doSetOption("Key1", QVariant("value1"));
    qBackend.doSetOption("Key2", QVariant("value2"));
    qBackend.doSetOption("Key3", QVariant(123));
    ASSERT_EQ(qBackend.getOption("Key1").toString(), "value1");
    ASSERT_EQ(qBackend.getOption("Key2").toString(), "value2");
    ASSERT_EQ(qBackend.getOption("Key3").toInt(), 123);
}

TEST_F(ut_QSettingsBackend, testDestructor)
{
    QString tmpFile = "/tmp/test_destructor.ini";
    QFile::remove(tmpFile);
    {
        QSettingBackend qBackend(tmpFile);
        qBackend.doSetOption("DestroyTest", QVariant("data"));
    }
    QSettingBackend qBackend2(tmpFile);
    ASSERT_EQ(qBackend2.getOption("DestroyTest").toString(), "data");
    QFile::remove(tmpFile);
}
