// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "backend/dsettingsdconfigbackend.h"
#include "settings/dsettings.h"

#include <gtest/gtest.h>
#include <QVariant>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "test_helper.hpp"

DCORE_USE_NAMESPACE

static constexpr char const *FILE_NAME = "example";

class ut_DSettingsDConfigBackend : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        fileBackendLocalPrefix.set("DSG_DCONFIG_FILE_BACKEND_LOCAL_PREFIX", "/tmp/example_dsettings");
        backendType.set("DSG_DCONFIG_BACKEND_TYPE", "FileBackend");
        dsgDataDir.set("DSG_DATA_DIRS", PREFIX "/share/dsg");
    }

    static void TearDownTestCase()
    {
        QDir(fileBackendLocalPrefix.value()).removeRecursively();
        fileBackendLocalPrefix.restore();
        backendType.restore();
        dsgDataDir.restore();
    }

    virtual void SetUp() override
    {
        // Check DConfig DBus session availability
        if (!QDBusConnection::sessionBus().isConnected() ||
            !QDBusConnection::systemBus().isConnected()) {
            GTEST_SKIP() << "DConfig DBus session not available";
        }
        metaFilePath = QString("%1" PREFIX "/share/dsg/configs/%2.json")
                           .arg(fileBackendLocalPrefix.value(), FILE_NAME);
        guard = new FileCopyGuard(":/data/dconf-example.meta.json", metaFilePath);
    }

    virtual void TearDown() override
    {
        delete guard;
        QDir(fileBackendLocalPrefix.value()).removeRecursively();
    }

    static EnvGuard backendType;
    static EnvGuard fileBackendLocalPrefix;
    static EnvGuard dsgDataDir;
    QString metaFilePath;
    FileCopyGuard *guard = nullptr;
};

EnvGuard ut_DSettingsDConfigBackend::fileBackendLocalPrefix;
EnvGuard ut_DSettingsDConfigBackend::backendType;
EnvGuard ut_DSettingsDConfigBackend::dsgDataDir;

TEST_F(ut_DSettingsDConfigBackend, keys)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    QStringList keyList = backend.keys();
    EXPECT_FALSE(keyList.isEmpty());
    EXPECT_TRUE(keyList.contains(QStringLiteral("canExit")));
    EXPECT_TRUE(keyList.contains(QStringLiteral("key2")));
}

TEST_F(ut_DSettingsDConfigBackend, getOptionExisting)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    QVariant val = backend.getOption(QStringLiteral("key2"));
    EXPECT_TRUE(val.isValid());
    EXPECT_EQ(val.toString(), QStringLiteral("125"));
}

TEST_F(ut_DSettingsDConfigBackend, getOptionNonExistentKey)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    QVariant val = backend.getOption(QStringLiteral("nonexistent_key_xyz"));
    EXPECT_FALSE(val.isValid());
}

TEST_F(ut_DSettingsDConfigBackend, getOptionEmptyKey)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    QVariant val = backend.getOption(QString());
    EXPECT_FALSE(val.isValid());
}

TEST_F(ut_DSettingsDConfigBackend, doSetOption)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    backend.doSetOption(QStringLiteral("key2"), QVariant(QStringLiteral("newValue")));

    QVariant val = backend.getOption(QStringLiteral("key2"));
    EXPECT_TRUE(val.isValid());
    EXPECT_EQ(val.toString(), QStringLiteral("newValue"));
}

TEST_F(ut_DSettingsDConfigBackend, doSetOptionMultiple)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    backend.doSetOption(QStringLiteral("key2"), QVariant(QStringLiteral("val1")));
    backend.doSetOption(QStringLiteral("canExit"), QVariant(false));
    backend.doSetOption(QStringLiteral("key3"), QVariant(QStringLiteral("val3")));

    EXPECT_EQ(backend.getOption(QStringLiteral("key2")).toString(), QStringLiteral("val1"));
    EXPECT_EQ(backend.getOption(QStringLiteral("canExit")).toBool(), false);
    EXPECT_EQ(backend.getOption(QStringLiteral("key3")).toString(), QStringLiteral("val3"));
}

TEST_F(ut_DSettingsDConfigBackend, doSetOptionEmptyKey)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    backend.doSetOption(QString(), QVariant(QStringLiteral("val")));
    // Setting an empty key should not crash; getOption with empty key returns invalid
    EXPECT_FALSE(backend.getOption(QString()).isValid());
}

TEST_F(ut_DSettingsDConfigBackend, doSetOptionInvalidValue)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    backend.doSetOption(QStringLiteral("key2"), QVariant());

    QVariant val = backend.getOption(QStringLiteral("key2"));
    // Setting invalid QVariant removes the cached value, so getOption falls through
    // to the config metadata default ("125")
    EXPECT_TRUE(val.isValid());
    EXPECT_EQ(val.toString(), QStringLiteral("125"));
}

TEST_F(ut_DSettingsDConfigBackend, setOptionViaSignal)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    QSignalSpy spy(&backend, &DSettingsBackend::optionChanged);

    Q_EMIT backend.setOption(QStringLiteral("key2"), QVariant(QStringLiteral("viaSignal")));
    QCoreApplication::processEvents();

    // doSetOption is invoked via QueuedConnection after processEvents
    QVariant val = backend.getOption(QStringLiteral("key2"));
    EXPECT_TRUE(val.isValid());
    EXPECT_EQ(val.toString(), QStringLiteral("viaSignal"));
}

TEST_F(ut_DSettingsDConfigBackend, syncViaSignal)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    // doSync is empty (TODO), but the signal-slot path should be exercised
    Q_EMIT backend.sync();
    QCoreApplication::processEvents();
    // Verify backend still functional after sync signal
    EXPECT_FALSE(backend.keys().isEmpty());
}

TEST_F(ut_DSettingsDConfigBackend, doSyncDirectCall)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    // doSync is currently empty (TODO in source); call directly to exercise code path
    backend.doSync();
    // Verify backend still functional after sync
    QStringList keyList = backend.keys();
    EXPECT_FALSE(keyList.isEmpty());
}

TEST_F(ut_DSettingsDConfigBackend, doSyncMultipleCalls)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    backend.doSync();
    backend.doSync();
    backend.doSync();
    // No crash; verify backend still functional
    EXPECT_EQ(backend.getOption(QStringLiteral("key2")).toString(), QStringLiteral("125"));
}

TEST_F(ut_DSettingsDConfigBackend, doSetOptionAndSyncSequence)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    backend.doSetOption(QStringLiteral("key2"), QVariant(QStringLiteral("seqVal")));
    backend.doSync();

    QVariant val = backend.getOption(QStringLiteral("key2"));
    EXPECT_EQ(val.toString(), QStringLiteral("seqVal"));
}

TEST_F(ut_DSettingsDConfigBackend, getOptionAfterSetOption)
{
    DSettingsDConfigBackend backend(FILE_NAME);
    // Verify initial value
    EXPECT_EQ(backend.getOption(QStringLiteral("key2")).toString(), QStringLiteral("125"));

    // Set new value
    backend.doSetOption(QStringLiteral("key2"), QVariant(QStringLiteral("updated")));
    // Verify getOption reflects the change
    EXPECT_EQ(backend.getOption(QStringLiteral("key2")).toString(), QStringLiteral("updated"));
}

TEST_F(ut_DSettingsDConfigBackend, integrationWithDSettings)
{
    static const char *jsonStr = R"({
        "groups": [{
            "key": "base",
            "name": "Basic",
            "options": [{
                "key": "key2",
                "name": "Key2",
                "type": "lineedit",
                "default": "125"
            }]
        }]
    })";

    auto settings = DSettings::fromJson(QByteArray(jsonStr));
    ASSERT_NE(settings, nullptr);

    auto backend = new DSettingsDConfigBackend(FILE_NAME);
    settings->setBackend(backend);

    // Verify integration: DSettings reads initial values from backend
    EXPECT_EQ(settings->value(QStringLiteral("base.key2")).toString(), QStringLiteral("125"));

    // Set option via DSettings. Note: DSettings prefixes the group key ("base") to the
    // option key, so the backend receives doSetOption("base.key2", "changed"). Since the
    // backend's DConfig only knows "key2" (not "base.key2"), the setOption does not change
    // the backend's "key2" value — getOption("key2") still returns the default "125".
    settings->setOption(QStringLiteral("base.key2"), QVariant(QStringLiteral("changed")));
    QCoreApplication::processEvents();

    // Backend "key2" is unchanged because the option key mismatch ("base.key2" vs "key2")
    EXPECT_EQ(backend->getOption(QStringLiteral("key2")).toString(), QStringLiteral("125"));

    delete settings.data();

    // Thread is stopped after settings destruction; safe to delete backend
    delete backend;
}
