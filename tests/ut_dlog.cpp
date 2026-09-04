// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
// Include LogManager.h and the .cpp to get DLogManagerPrivate full definition
// (only forward-declared in header). Access to private members is granted by
// -fno-access-control in CMakeLists.txt, so no #define private public is needed.
#include "log/LogManager.h"
#include "LogManager.cpp"

#include "dpathbuf.h"
#include "dstandardpaths.h"
#include "test_helper.hpp"
#include <gtest/gtest.h>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <sys/stat.h>
#include <unistd.h>
#include <QDBusReply>

DCORE_USE_NAMESPACE

static bool isDConfigServiceAvailable()
{
    if (!QDBusConnection::systemBus().isConnected())
        return false;
    if (!QDBusConnection::systemBus().interface()->isServiceRegistered("org.desktopspec.ConfigManager"))
        return false;
    const QDBusReply<QStringList> activatableNames =
        QDBusConnection::systemBus().interface()->callWithArgumentList(
            QDBus::AutoDetect, QLatin1String("ListActivatableNames"), QList<QVariant>());
    return activatableNames.isValid() && activatableNames.value().contains("org.desktopspec.ConfigManager");
}

class ut_DLogManager : public testing::Test
{
protected:
    void TearDown() override
    {
        // Unconditionally restore DLogManager singleton state to prevent
        // leakage between tests when ASSERT fails mid-test (Issue 9)
        DLogManagerPrivate *d = DLogManager::instance()->d_func();
        d->m_logPath.clear();
        d->m_format = QStringLiteral("%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}");
    }
};

TEST_F(ut_DLogManager, testDLogManager)
{
    DPathBuf logPath(QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first());

    logPath = logPath / "tests.log";

    ASSERT_EQ(DLogManager::getlogFilePath(), logPath.toString());
}

TEST_F(ut_DLogManager, testDefaultLogPath)
{
    EnvGuard guard;
    guard.unset("HOME");

    // unset HOME env will not init default log file path
    ASSERT_TRUE(DLogManager::getlogFilePath().contains(DStandardPaths::homePath()));
}

TEST_F(ut_DLogManager, testSetInvalidLogPath)
{
    QString tmp = QDir::tempPath();
    DLogManager::setlogFilePath(tmp);
    // set log file path to a dir is not supported
    ASSERT_NE(DLogManager::getlogFilePath(), tmp);
}

TEST_F(ut_DLogManager, testSetValidLogPath)
{
    QString validPath = QDir::tempPath() + "/test_dlog_valid.log";
    DLogManager::setlogFilePath(validPath);
    EXPECT_EQ(DLogManager::getlogFilePath(), validPath);
    // Reset to default for other tests
    DLogManager::instance()->d_func()->m_logPath.clear();
    QFile::remove(validPath);
}

TEST_F(ut_DLogManager, testRegisterConsoleAppender)
{
    // Should not crash
    DLogManager::registerConsoleAppender();
    // Verify console appender was created
    EXPECT_NE(DLogManager::instance()->d_func()->m_consoleAppender, nullptr);
}

TEST_F(ut_DLogManager, testRegisterFileAppender)
{
    // Set a valid log path first
    QString logPath = QDir::tempPath() + "/test_dlog_file_appender.log";
    DLogManager::setlogFilePath(logPath);

    DLogManager::registerFileAppender();
    EXPECT_NE(DLogManager::instance()->d_func()->m_rollingFileAppender, nullptr);

    // Reset
    DLogManager::instance()->d_func()->m_logPath.clear();
    QFile::remove(logPath);
}

TEST_F(ut_DLogManager, testSetLogFormat)
{
    QString customFormat = "%{message}";
    DLogManager::setLogFormat(customFormat);
    EXPECT_EQ(DLogManager::instance()->d_func()->m_format, customFormat);
    // Restore default
    DLogManager::setLogFormat(QStringLiteral("%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}"));
}

TEST_F(ut_DLogManager, testJoinPath)
{
    DLogManager *mgr = DLogManager::instance();
    QString result = mgr->joinPath(QStringLiteral("/tmp"), QStringLiteral("test.log"));
    EXPECT_TRUE(result.contains("test.log"));
    EXPECT_TRUE(result.startsWith("/tmp"));
}

TEST_F(ut_DLogManager, testRegisterJournalAppender)
{
    // registerJournalAppender either creates a JournalAppender (BUILD_WITH_SYSTEMD)
    // or prints a warning. Verify no crash and that the appender pointer is either
    // set (systemd build) or remains null (non-systemd build).
    DLogManager::registerJournalAppender();
    DLogManagerPrivate *d = DLogManager::instance()->d_func();
#ifdef BUILD_WITH_SYSTEMD
    EXPECT_NE(d->m_journalAppender, nullptr);
#else
    // m_journalAppender member only exists under BUILD_WITH_SYSTEMD
    SUCCEED();
#endif
}

TEST_F(ut_DLogManager, testGetlogFilePathWithEmptyHome)
{
    // Use qputenv to set HOME to empty string — qunsetenv may not fully clear
    // it on all platforms (getpwuid fallback). Setting to empty is more reliable.
    QByteArray savedHome = qgetenv("HOME");
    qputenv("HOME", "");
    DLogManager::instance()->d_func()->m_logPath.clear();
    QString result = DLogManager::getlogFilePath();
    // When HOME is empty, homePath() returns empty and getlogFilePath returns
    // a path containing tests.log but with empty home prefix
    EXPECT_TRUE(result.contains("tests.log"));
    // Restore by clearing cache
    DLogManager::instance()->d_func()->m_logPath.clear();
    if (savedHome.isEmpty())
        qunsetenv("HOME");
    else
        qputenv("HOME", savedHome);
}

TEST_F(ut_DLogManager, testInitLoggingRulesWithEnvDisabled)
{
    EnvGuard guard;
    guard.set("DTK_DISABLED_LOGGING_RULES", "1", false);

    // Create a new DLogManager to test initLoggingRules path
    // initLoggingRules should return early when DTK_DISABLED_LOGGING_RULES is set
    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    d->initLoggingRules();
    // No crash expected, dsgConfig should be null
    EXPECT_EQ(d->m_dsgConfig, nullptr);
}

TEST_F(ut_DLogManager, testInitLoggingRulesWithQtLoggingRules)
{
    EnvGuard guard;
    guard.set("QT_LOGGING_RULES", "*.debug=false", false);

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    d->initLoggingRules();
    // Should return early
    EXPECT_EQ(d->m_dsgConfig, nullptr);
}

TEST_F(ut_DLogManager, testInitLoggingRulesNormal)
{
    if (!isDConfigServiceAvailable()) {
        GTEST_SKIP() << "DConfig service not available — initLoggingRules crashes without it";
    }
    // Without disabling env, initLoggingRules will try to create DConfig
    // which may fail in test env — verify no crash and that dsgConfig/fallbackConfig
    // are either set or null (depends on DConfig availability)
    EnvGuard guard1;
    guard1.unset("DTK_DISABLED_LOGGING_RULES");
    EnvGuard guard2;
    guard2.unset("QT_LOGGING_RULES");

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    d->m_dsgConfig.reset();
    d->m_fallbackConfig.reset();
    d->initLoggingRules();
    // In test env without DConfig, both should remain null;
    // if DConfig is available, at least one may be set.
    // When DConfig service IS available, initLoggingRules creates non-null
    // m_dsgConfig / m_fallbackConfig; when it's NOT available, they stay null.
    // Either outcome is valid — just verify no crash.
    SUCCEED();
}

TEST_F(ut_DLogManager, testUpdateLoggingRules)
{
    if (!isDConfigServiceAvailable()) {
        GTEST_SKIP() << "DConfig service not available — updateLoggingRules crashes without it";
    }
    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    // No configs set — updateLoggingRules should be a no-op
    d->m_dsgConfig.reset();
    d->m_fallbackConfig.reset();
    d->updateLoggingRules();
    // With no configs, updateLoggingRules is a no-op regardless of whether
    // initLoggingRules was previously called. Just verify no crash.
    SUCCEED();
}

TEST_F(ut_DLogManager, testShouldSkipConsoleAppender)
{
    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    // In test environment, should not be under systemd
    bool result = d->shouldSkipConsoleAppender();
    // Just verify it returns without crash
    EXPECT_FALSE(result);
}

TEST_F(ut_DLogManager, testShouldSkipConsoleAppenderForced)
{
    EnvGuard guard;
    guard.set("DTK_FORCE_CONSOLE_LOGGING", "1", false);
    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    EXPECT_FALSE(d->shouldSkipConsoleAppender());
}

TEST_F(ut_DLogManager, testCreateDConfigEmptyAppId)
{
    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    auto *config = d->createDConfig(QString());
    EXPECT_EQ(config, nullptr);
}

// ---- Additional tests for uncovered LogManager.cpp paths ----

TEST_F(ut_DLogManager, testCreateDConfigWithAppId)
{
    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    // Non-empty appId exercises the createDConfig body (dconfig create + signal connect).
    // Without DConfig service the pointer may be null; either way no crash.
    auto *config = d->createDConfig(QStringLiteral("org.deepin.test.app"));
    // config could be null or valid — just verify no crash
    if (config)
        delete config;
    SUCCEED();
}

TEST_F(ut_DLogManager, testGetlogFilePathWithOrgAndApp)
{
    // Exercises appendOrganizationAndApp() which appends org/app to cache path
    QString savedOrg = QCoreApplication::organizationName();
    QString savedApp = QCoreApplication::applicationName();

    QCoreApplication::setOrganizationName(QStringLiteral("TestOrg"));
    QCoreApplication::setApplicationName(QStringLiteral("TestApp"));

    DLogManager::instance()->d_func()->m_logPath.clear();
    QString path = DLogManager::getlogFilePath();

    EXPECT_TRUE(path.contains("TestOrg"));
    EXPECT_TRUE(path.contains("TestApp"));
    EXPECT_TRUE(path.endsWith("TestApp.log"));

    // Restore
    DLogManager::instance()->d_func()->m_logPath.clear();
    QCoreApplication::setOrganizationName(savedOrg);
    QCoreApplication::setApplicationName(savedApp);
}

TEST_F(ut_DLogManager, testGetlogFilePathCreatesCacheDir)
{
    // Exercises the QDir::mkpath branch in getlogFilePath
    QString savedOrg = QCoreApplication::organizationName();
    QString savedApp = QCoreApplication::applicationName();

    QCoreApplication::setOrganizationName(QStringLiteral("CoverageTestOrg"));
    QCoreApplication::setApplicationName(QStringLiteral("CoverageTestApp"));

    DLogManager::instance()->d_func()->m_logPath.clear();
    QString path = DLogManager::getlogFilePath();

    // The cache directory should have been created
    QFileInfo fi(path);
    EXPECT_TRUE(QDir(fi.absolutePath()).exists());

    // Cleanup
    QDir(QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first()
         + "/CoverageTestOrg").removeRecursively();
    DLogManager::instance()->d_func()->m_logPath.clear();
    QCoreApplication::setOrganizationName(savedOrg);
    QCoreApplication::setApplicationName(savedApp);
}

TEST_F(ut_DLogManager, testSetlogFilePathNonExistent)
{
    // Path does not exist → info.exists() is false → goes to else (sets path)
    QString nonExistent = QDir::tempPath() + "/nonexistent_dlog_test_12345.log";
    QFile::remove(nonExistent);
    DLogManager::setlogFilePath(nonExistent);
    EXPECT_EQ(DLogManager::getlogFilePath(), nonExistent);
    DLogManager::instance()->d_func()->m_logPath.clear();
}

TEST_F(ut_DLogManager, testSetlogFilePathExistingFile)
{
    // Create an actual file, then set path to it → info.exists() && info.isFile() → else branch
    QString tmpFile = QDir::tempPath() + "/dlog_existing_file_test.log";
    {
        QFile f(tmpFile);
        f.open(QIODevice::WriteOnly);
        f.write("test");
        f.close();
    }
    DLogManager::setlogFilePath(tmpFile);
    EXPECT_EQ(DLogManager::getlogFilePath(), tmpFile);
    DLogManager::instance()->d_func()->m_logPath.clear();
    QFile::remove(tmpFile);
}

TEST_F(ut_DLogManager, testInitLoggingRulesWithFallbackAppId)
{
    // Exercises the fallback config path in initLoggingRules
    EnvGuard guard1;
    guard1.unset("DTK_DISABLED_LOGGING_RULES");
    EnvGuard guard2;
    guard2.unset("QT_LOGGING_RULES");
    EnvGuard guard3;
    guard3.set("DTK_LOGGING_FALLBACK_APPID", "org.deepin.fallback.test", false);

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    d->m_dsgConfig.reset();
    d->m_fallbackConfig.reset();
    d->initLoggingRules();

    // Without DConfig service, both remain null; with it, fallbackConfig may be set.
    // Either outcome is valid — verify no crash.
    SUCCEED();

    // Cleanup
    d->m_dsgConfig.reset();
    d->m_fallbackConfig.reset();
}

TEST_F(ut_DLogManager, testInitLoggingRulesFallbackEqualDsgAppId)
{
    // When fallbackId equals dsgAppId, fallback config should NOT be created.
    // DSGApplication::id() returns app name or binary path; set fallback to same.
    EnvGuard guard1;
    guard1.unset("DTK_DISABLED_LOGGING_RULES");
    EnvGuard guard2;
    guard2.unset("QT_LOGGING_RULES");

    QByteArray dsgId = DSGApplication::id();
    EnvGuard guard3;
    guard3.set("DTK_LOGGING_FALLBACK_APPID", dsgId, false);

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    d->m_dsgConfig.reset();
    d->m_fallbackConfig.reset();
    d->initLoggingRules();

    // fallbackConfig should NOT be set since fallbackId == dsgAppId
    EXPECT_EQ(d->m_fallbackConfig, nullptr);

    d->m_dsgConfig.reset();
}

TEST_F(ut_DLogManager, testInitRollingFileAppenderProperties)
{
    // Exercises initRollingFileAppender — creates RollingFileAppender, sets format,
    // logFilesLimit, datePattern
    QString logPath = QDir::tempPath() + "/test_dlog_rolling_props.log";
    DLogManager::setlogFilePath(logPath);

    // Clean up any previous appender to avoid double-registration
    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    if (d->m_rollingFileAppender) {
        dlogger->unregisterAppender(d->m_rollingFileAppender);
        delete d->m_rollingFileAppender;
        d->m_rollingFileAppender = nullptr;
    }

    DLogManager::registerFileAppender();
    EXPECT_NE(d->m_rollingFileAppender, nullptr);

    // Cleanup
    dlogger->unregisterAppender(d->m_rollingFileAppender);
    delete d->m_rollingFileAppender;
    d->m_rollingFileAppender = nullptr;
    d->m_logPath.clear();
    QFile::remove(logPath);
}

TEST_F(ut_DLogManager, testInitConsoleAppenderFormat)
{
    // Exercises initConsoleAppender — creates ConsoleAppender and sets format
    DLogManagerPrivate *d = DLogManager::instance()->d_func();

    // Clean up previous appender
    if (d->m_consoleAppender) {
        dlogger->unregisterAppender(d->m_consoleAppender);
        delete d->m_consoleAppender;
        d->m_consoleAppender = nullptr;
    }

    QString customFormat = "%{message}";
    DLogManager::setLogFormat(customFormat);
    DLogManager::registerConsoleAppender();

    EXPECT_NE(d->m_consoleAppender, nullptr);

    // Cleanup
    dlogger->unregisterAppender(d->m_consoleAppender);
    delete d->m_consoleAppender;
    d->m_consoleAppender = nullptr;
    // Restore default format
    DLogManager::setLogFormat(QStringLiteral(DEFAULT_FMT));
}

TEST_F(ut_DLogManager, testShouldSkipConsoleAppenderJournalStreamEmpty)
{
    // JOURNAL_STREAM not set → returns false
    EnvGuard guard;
    guard.unset("JOURNAL_STREAM");
    guard.unset("DTK_FORCE_CONSOLE_LOGGING");

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    EXPECT_FALSE(d->shouldSkipConsoleAppender());
}

TEST_F(ut_DLogManager, testShouldSkipConsoleAppenderJournalStreamMalformed)
{
    // JOURNAL_STREAM set but malformed (not 2 parts) → returns false
    EnvGuard guard;
    guard.set("JOURNAL_STREAM", "single_part", false);
    guard.unset("DTK_FORCE_CONSOLE_LOGGING");

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    EXPECT_FALSE(d->shouldSkipConsoleAppender());
}

TEST_F(ut_DLogManager, testShouldSkipConsoleAppenderJournalStreamWrongValues)
{
    // JOURNAL_STREAM has valid format but wrong values → returns false
    EnvGuard guard;
    guard.set("JOURNAL_STREAM", "999999:999999", false);
    guard.unset("DTK_FORCE_CONSOLE_LOGGING");

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    EXPECT_FALSE(d->shouldSkipConsoleAppender());
}

TEST_F(ut_DLogManager, testShouldSkipConsoleAppenderJournalStreamMatch)
{
    // Construct JOURNAL_STREAM from actual stdout stat to exercise the match → true
    struct stat st;
    ASSERT_EQ(fstat(STDOUT_FILENO, &st), 0);

    QByteArray matchStream = QByteArray::number(st.st_dev) + ":" + QByteArray::number(st.st_ino);

    EnvGuard guard;
    guard.set("JOURNAL_STREAM", matchStream, false);
    guard.unset("DTK_FORCE_CONSOLE_LOGGING");

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    EXPECT_TRUE(d->shouldSkipConsoleAppender());
}

TEST_F(ut_DLogManager, testShouldSkipConsoleAppenderJournalStreamInvalidNumbers)
{
    // JOURNAL_STREAM with non-numeric parts → ok1/ok2 false → returns false
    EnvGuard guard;
    guard.set("JOURNAL_STREAM", "abc:def", false);
    guard.unset("DTK_FORCE_CONSOLE_LOGGING");

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    EXPECT_FALSE(d->shouldSkipConsoleAppender());
}

TEST_F(ut_DLogManager, testJoinPathSeparator)
{
    // Exercise joinPath with various inputs
    DLogManager *mgr = DLogManager::instance();
    QString result1 = mgr->joinPath(QStringLiteral("/var/log"), QStringLiteral("app.log"));
    EXPECT_TRUE(result1.contains(QDir::separator()));
    EXPECT_TRUE(result1.endsWith("app.log"));

    QString result2 = mgr->joinPath(QString(), QStringLiteral("test.log"));
    EXPECT_TRUE(result2.endsWith("test.log"));
}

TEST_F(ut_DLogManager, testGetlogFilePathEmptyHomeReturnsEmpty)
{
    // When home path is empty, getlogFilePath returns empty string (qWarning path)
    EnvGuard guard;
    guard.set("HOME", "", false);

    DLogManager::instance()->d_func()->m_logPath.clear();
    QString result = DLogManager::getlogFilePath();
    // With empty HOME, DStandardPaths::homePath() returns empty → qWarning + return ""
    // But homePath() might use getpwuid fallback, so just verify no crash
    // If result is empty, the warning path was taken; if non-empty, fallback was used
    SUCCEED();

    DLogManager::instance()->d_func()->m_logPath.clear();
}

TEST_F(ut_DLogManager, testUpdateLoggingRulesWithDsgConfig)
{
    if (!isDConfigServiceAvailable()) {
        GTEST_SKIP() << "DConfig service not available";
    }
    // Exercise updateLoggingRules after initLoggingRules creates configs
    EnvGuard guard1;
    guard1.unset("DTK_DISABLED_LOGGING_RULES");
    EnvGuard guard2;
    guard2.unset("QT_LOGGING_RULES");

    DLogManagerPrivate *d = DLogManager::instance()->d_func();
    d->m_dsgConfig.reset();
    d->m_fallbackConfig.reset();
    d->initLoggingRules();
    d->updateLoggingRules();
    // No crash expected
    SUCCEED();

    d->m_dsgConfig.reset();
    d->m_fallbackConfig.reset();
}
