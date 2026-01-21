// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later

// Test: dconfig_org_deepin_dtk_preference crash stability
// This test verifies the thread-safe generated DConfig class handles:
// 1. Immediate destruction during initialization
// 2. Destruction after successful initialization
// 3. Destruction during active initialization (race condition)
// 4. Signal thread affinity and correctness
// 5. Property change operations under stress
// 6. Concurrent config instances

#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QEventLoop>
#include <QGuiApplication>
#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <QThread>

// Include the generated DConfig class
#include "dconfig_org_deepin_dtk_preference.hpp"

class ut_dconfig_org_deepin_dtk_preference : public testing::Test
{
protected:
    // Test configuration constants - reduced for faster execution
    static constexpr int IMMEDIATE_DESTROY_CYCLES = 10;
    static constexpr int POST_INIT_DESTROY_CYCLES = 5;
    static constexpr int MID_INIT_DESTROY_CYCLES = 20;
    static constexpr int PROPERTY_CHANGE_CYCLES = 5;
    static constexpr int RAPID_CHANGE_CYCLES = 3;
    static constexpr int CONCURRENT_CONFIG_CYCLES = 3;

    static constexpr int INIT_TIMEOUT_MS = 2000;  // Total timeout for initialization

    static constexpr const char *DSG_CONFIG_SERVICE = "org.desktopspec.ConfigManager";

    // Static flag to track if DConfig service is available
    static bool s_dConfigAvailable;

    // Helper to create config
    dconfig_org_deepin_dtk_preference *createConfig()
    {
        return dconfig_org_deepin_dtk_preference::create(
            QStringLiteral("org.deepin.dtk.preference"),
            QStringLiteral("/test"));
    }

    // Check if DConfig service is available using DBus
    static bool isDConfigServiceAvailable()
    {
        // Check if service is registered
        if (!QDBusConnection::systemBus().interface()->isServiceRegistered(DSG_CONFIG_SERVICE)) {
            return false;
        }

        // Check if service is activatable (similar to DConfig::isServiceActivatable)
        const QDBusReply<QStringList> activatableNames =
            QDBusConnection::systemBus().interface()->callWithArgumentList(
                QDBus::AutoDetect,
                QLatin1String("ListActivatableNames"),
                QList<QVariant>());

        return activatableNames.isValid() && activatableNames.value().contains(DSG_CONFIG_SERVICE);
    }

    static void SetUpTestSuite()
    {
        // Quick check using DBus instead of creating actual config
        s_dConfigAvailable = isDConfigServiceAvailable();
    }
};

// Static member initialization
bool ut_dconfig_org_deepin_dtk_preference::s_dConfigAvailable = false;

// Test 1: Simple creation and immediate deletion
TEST_F(ut_dconfig_org_deepin_dtk_preference, test_immediate_destroy)
{
    if (!s_dConfigAvailable) {
        GTEST_SKIP() << "DConfig service not available on this system";
    }

    for (int cycle = 0; cycle < IMMEDIATE_DESTROY_CYCLES; ++cycle) {
        auto config = createConfig();

        // Delete immediately without waiting for initialization
        delete config;
        QCoreApplication::processEvents();
    }
}

// Test 2: Destroy after signal connection is established
TEST_F(ut_dconfig_org_deepin_dtk_preference, test_destroy_after_initialization)
{
    if (!s_dConfigAvailable) {
        GTEST_SKIP() << "DConfig service not available on this system";
    }

    for (int cycle = 0; cycle < POST_INIT_DESTROY_CYCLES; ++cycle) {
        auto config = createConfig();

        // Wait for initialization
        bool initialized = QTest::qWaitFor([config]() {
            return config->isInitializeSucceeded();
        }, INIT_TIMEOUT_MS);
        
        if (initialized) {
            auto dconfig = config->config();
            EXPECT_TRUE(dconfig->isValid()) << "DConfig is not valid in cycle " << cycle;
        }

        delete config;
    }
}

// Test 3: Destroy DURING initialization (mid-initialization race condition)
TEST_F(ut_dconfig_org_deepin_dtk_preference, test_destroy_during_initialization)
{
    if (!s_dConfigAvailable) {
        GTEST_SKIP() << "DConfig service not available on this system";
    }

    for (int cycle = 0; cycle < MID_INIT_DESTROY_CYCLES; ++cycle) {
        auto config = createConfig();

        // Delete after a very short delay to hit the initialization window
        int delayMicros = (cycle % 10) * 100;  // 0-900 microseconds
        if (delayMicros > 0) {
            QCoreApplication::processEvents();
            QThread::usleep(delayMicros);
        }

        delete config;
        QCoreApplication::processEvents();
    }
}

// Test 4: Verify signal thread affinity
TEST_F(ut_dconfig_org_deepin_dtk_preference, test_signal_thread_affinity)
{
    if (!s_dConfigAvailable) {
        GTEST_SKIP() << "DConfig service not available on this system";
    }

    auto config = createConfig();

    QThread *mainThread = QThread::currentThread();
    QThread *signalThread = nullptr;

    QSignalSpy spyAutoDisplay(config, &dconfig_org_deepin_dtk_preference::autoDisplayFeatureChanged);

    // Connect to verify thread affinity
    QObject::connect(config, &dconfig_org_deepin_dtk_preference::autoDisplayFeatureChanged, 
            qApp, [&]() {
        signalThread = QThread::currentThread();
    }, Qt::DirectConnection);

    // Wait for initialization
    ASSERT_TRUE(QTest::qWaitFor([config]() {
        return config->isInitializeSucceeded();
    }, INIT_TIMEOUT_MS)) << "Config failed to initialize";

    auto dconfig = config->config();
    ASSERT_TRUE(dconfig->isValid()) << "DConfig is not valid";

    // Trigger property change
    bool currentValue = config->autoDisplayFeature();
    config->setAutoDisplayFeature(!currentValue);

    // Wait for signal using QSignalSpy
    EXPECT_EQ(spyAutoDisplay.count(), 1) << "autoDisplayFeatureChanged signal not emitted";
    EXPECT_EQ(signalThread, mainThread) << "Signal emitted in wrong thread";

    delete config;
}

// Test 5: Trigger property change then destroy
TEST_F(ut_dconfig_org_deepin_dtk_preference, test_property_change_then_destroy)
{
    if (!s_dConfigAvailable) {
        GTEST_SKIP() << "DConfig service not available on this system";
    }

    for (int cycle = 0; cycle < PROPERTY_CHANGE_CYCLES; ++cycle) {
        auto config = createConfig();

        if (!QTest::qWaitFor([config]() {
            return config->isInitializeSucceeded();
        }, INIT_TIMEOUT_MS)) {
            delete config;
            continue;
        }

        auto dconfig = config->config();
        EXPECT_TRUE(dconfig->isValid()) << "DConfig is not valid in cycle " << cycle;

        // Change a property and immediately delete
        config->setColorMode(QString("test_theme_%1").arg(cycle));
        
        delete config;
    }
}

// Test 6: Rapid property changes then destroy
TEST_F(ut_dconfig_org_deepin_dtk_preference, test_rapid_changes_then_destroy)
{
    if (!s_dConfigAvailable) {
        GTEST_SKIP() << "DConfig service not available on this system";
    }

    for (int cycle = 0; cycle < RAPID_CHANGE_CYCLES; ++cycle) {
        auto config = createConfig();

        if (!QTest::qWaitFor([config]() {
            return config->isInitializeSucceeded();
        }, INIT_TIMEOUT_MS)) {
            delete config;
            continue;
        }

        auto dconfig = config->config();
        EXPECT_TRUE(dconfig->isValid()) << "DConfig is not valid in cycle " << cycle;

        // Rapidly change multiple properties
        for (int j = 0; j < 5; ++j) {
            config->setColorMode(QString("theme_%1").arg(j));
            config->setDefaultColorMode(QString("color_%1").arg(j));
            QTest::qWait(1);
        }

        delete config;
    }
}

// Test 7: Concurrent multiple configs
TEST_F(ut_dconfig_org_deepin_dtk_preference, test_concurrent_configs)
{
    if (!s_dConfigAvailable) {
        GTEST_SKIP() << "DConfig service not available on this system";
    }

    for (int cycle = 0; cycle < CONCURRENT_CONFIG_CYCLES; ++cycle) {
        QList<dconfig_org_deepin_dtk_preference *> configs;

        // Create multiple configs at once
        for (int i = 0; i < 3; ++i) {
            auto config = createConfig();
            configs.append(config);
        }

        // Wait for all to initialize
        for (auto config : configs) {
            EXPECT_TRUE(QTest::qWaitFor([config]() {
                return config->isInitializeSucceeded();
            }, INIT_TIMEOUT_MS));
        }

        // Clean up all configs
        qDeleteAll(configs);
    }
}

// Test 8: Verify property getters and setters
TEST_F(ut_dconfig_org_deepin_dtk_preference, test_property_getters_and_setters)
{
    if (!s_dConfigAvailable) {
        GTEST_SKIP() << "DConfig service not available on this system";
    }

    auto config = createConfig();

    // Wait for initialization
    ASSERT_TRUE(QTest::qWaitFor([config]() {
        return config->isInitializeSucceeded();
    }, INIT_TIMEOUT_MS)) << "Config failed to initialize";

    // Test getters - should not crash
    bool autoDisplay = config->autoDisplayFeature();

    // Test setters
    config->setAutoDisplayFeature(!autoDisplay);

    // Use QSignalSpy to verify signals are emitted
    QSignalSpy spyAutoDisplay(config, &dconfig_org_deepin_dtk_preference::autoDisplayFeatureChanged);

    // Trigger changes
    config->setAutoDisplayFeature(autoDisplay);  // Set back to original

    // Wait for signals with timeout
    EXPECT_TRUE(QTest::qWaitFor([&spyAutoDisplay] () {
        return spyAutoDisplay.count() > 0;
    }, 1000)) << "autoDisplayFeatureChanged signal not emitted";

    delete config;
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ut_dconfig_org_deepin_dtk_preference);
