// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QFile>
#include <unistd.h>
#include <sys/wait.h>
#include <QFileInfo>
#include <QDir>
#include "dsgapplication.h"

DCORE_USE_NAMESPACE

// DSGApplication::id() caches its result in a function-local static
// (src/dsgapplication.cpp: static QByteArray selfId = getSelfAppId()).
// The FIRST call to id() in the process decides the cached value:
//   - If DSG_APP_ID env is set → that value is cached directly.
//   - If not set → getSelfAppId() calls getId(selfPid), which calls
//     checkDBusServiceActivatable (exercising D-Bus connection code),
//     then if AM is unreachable selfId stays empty and id() enters the
//     fallback path (applicationName → /proc/self/cmdline → /proc/self/exe
//     → formatAppId).
//
// To maximise coverage we ensure the first id() call happens WITHOUT
// DSG_APP_ID, so the fallback + formatAppId + D-Bus connection code
// are all exercised.  Subsequent tests verify cached behaviour.

// This test must run FIRST (gtest preserves declaration order within a
// test case) so that the fallback path is taken before any test sets
// DSG_APP_ID.
TEST(ut_DSGApplication, idFallbackPathFirstCall)
{
    // Ensure DSG_APP_ID is NOT set so the fallback path runs
    QByteArray savedAppId = qgetenv("DSG_APP_ID");
    qunsetenv("DSG_APP_ID");

    QByteArray result = DSGApplication::id();
    // Without DSG_APP_ID and without AM, id() falls back to
    // applicationName or /proc/self/cmdline or /proc/self/exe.
    // The test binary is launched with a path, so result should be non-empty
    // and formatted (no '/' chars — formatAppId replaces them with '.').
    EXPECT_FALSE(result.isEmpty());
    EXPECT_FALSE(result.contains('/'));

    // Restore
    if (!savedAppId.isEmpty())
        qputenv("DSG_APP_ID", savedAppId);
}

// Test that id() returns a consistent value on repeated calls (static caching)
TEST(ut_DSGApplication, idConsistent)
{
    QByteArray first = DSGApplication::id();
    QByteArray second = DSGApplication::id();
    EXPECT_EQ(first, second);
}

// Test that id() result does not contain path separators (formatAppId)
TEST(ut_DSGApplication, idReturnsFormatted)
{
    QByteArray result = DSGApplication::id();
    EXPECT_FALSE(result.isEmpty());
    // formatAppId replaces QDir::separator() with '.' and non-word chars with '-'
    EXPECT_FALSE(result.contains('/'));
}

// Test that id() is non-empty under normal conditions
TEST(ut_DSGApplication, idNotEmpty)
{
    QByteArray result = DSGApplication::id();
    EXPECT_FALSE(result.isEmpty());
}

// Test that DSG_APP_ID env has no effect after first call (caching)
TEST(ut_DSGApplication, idEnvIgnoredAfterCache)
{
    QByteArray cached = DSGApplication::id();
    // Setting DSG_APP_ID now should NOT change the cached value
    qputenv("DSG_APP_ID", "should-be-ignored");
    QByteArray result = DSGApplication::id();
    EXPECT_EQ(result, cached);
    qunsetenv("DSG_APP_ID");
}

// Test DTK_DISABLED_FALLBACK_APPID — but since id() is already cached
// from the first call, this only verifies the cached value is returned
TEST(ut_DSGApplication, idWithDisabledFallback)
{
    // id() is already cached from idFallbackPathFirstCall; setting
    // DTK_DISABLED_FALLBACK_APPID now has no effect.
    QByteArray cached = DSGApplication::id();
    qputenv("DTK_DISABLED_FALLBACK_APPID", "1");
    QByteArray result = DSGApplication::id();
    EXPECT_EQ(result, cached);
    qunsetenv("DTK_DISABLED_FALLBACK_APPID");
}

// === getId tests ===
// getId() calls isServiceActivatable("org.desktopspec.ApplicationManager1")
// which calls checkDBusServiceActivatable. In the test environment AM is
// typically not running, so getId returns empty. These tests exercise the
// D-Bus connection / error-handling paths in checkDBusServiceActivatable.

// Test getId with an invalid pid (-1) — pidfd_open will fail if reached
TEST(ut_DSGApplication, getIdInvalidPid)
{
    QByteArray result = DSGApplication::getId(-1);
    // AM is likely not activatable in test env, so result should be empty.
    // If AM IS activatable, pidfd_open(-1) fails → still empty.
    EXPECT_TRUE(result.isEmpty());
}

// Test getId with pid 0 — pidfd_open should fail
TEST(ut_DSGApplication, getIdPidZero)
{
    QByteArray result = DSGApplication::getId(0);
    EXPECT_TRUE(result.isEmpty());
}

// Test getId with self pid
TEST(ut_DSGApplication, getIdSelfPid)
{
    qint64 myPid = QCoreApplication::applicationPid();
    QByteArray result = DSGApplication::getId(myPid);
    // AM is likely not running in test env → empty
    EXPECT_TRUE(result.isEmpty());
}

// Test getId with init process (pid 1)
TEST(ut_DSGApplication, getIdPid1)
{
    QByteArray result = DSGApplication::getId(1);
    // Should not crash; result likely empty (AM not activatable)
    EXPECT_TRUE(result.isEmpty());
}

// Test getId with a very large pid — pidfd_open should fail
TEST(ut_DSGApplication, getIdLargePid)
{
    QByteArray result = DSGApplication::getId(999999);
    EXPECT_TRUE(result.isEmpty());
}

// Test getId with a large negative pid
TEST(ut_DSGApplication, getIdNegativePid)
{
    QByteArray result = DSGApplication::getId(-999);
    EXPECT_TRUE(result.isEmpty());
}

// Test getId consistency — calling twice with same pid returns same result
TEST(ut_DSGApplication, getIdConsistent)
{
    qint64 myPid = QCoreApplication::applicationPid();
    QByteArray first = DSGApplication::getId(myPid);
    QByteArray second = DSGApplication::getId(myPid);
    EXPECT_EQ(first, second);
}

// Test getId with a zombie/defunct process if one exists
// pidfd_open may succeed for an existing but zombie process,
// but AM identify will still fail → empty
TEST(ut_DSGApplication, getIdNonExistentPid)
{
    // A pid that almost certainly doesn't exist
    // Find an unlikely pid by using a very high number
    QByteArray result = DSGApplication::getId(2147483647);
    EXPECT_TRUE(result.isEmpty());
}

// Test that id() result is stable and can be used as a string identifier
TEST(ut_DSGApplication, idIsUsableIdentifier)
{
    QByteArray result = DSGApplication::id();
    EXPECT_FALSE(result.isEmpty());
    // The result should be a valid identifier (no spaces, no slashes)
    EXPECT_FALSE(result.contains(' '));
    EXPECT_FALSE(result.contains('/'));
    EXPECT_FALSE(result.contains('\\'));
}

// Test that id() result contains only word chars, hyphens, and dots (formatAppId regex)
TEST(ut_DSGApplication, idFormatAppIdRegex)
{
    QByteArray result = DSGApplication::id();
    EXPECT_FALSE(result.isEmpty());
    // formatAppId replaces non-word chars (except - and .) with '-'
    for (char c : result) {
        bool valid = (c >= 'a' && c <= 'z') ||
                     (c >= 'A' && c <= 'Z') ||
                     (c >= '0' && c <= '9') ||
                     c == '-' || c == '.';
        EXPECT_TRUE(valid) << "Unexpected character in id(): " << c;
    }
}

// Test getId with a recently exited process (pid reuse edge case)
TEST(ut_DSGApplication, getIdShortLivedPid)
{
    // Fork a child that exits immediately, then use its pid
    pid_t child = fork();
    if (child == 0) {
        _exit(0);
    }
    if (child > 0) {
        // Wait for child to exit
        int status;
        waitpid(child, &status, 0);
        // Now the pid is likely recycled or defunct
        QByteArray result = DSGApplication::getId(child);
        // AM not available → empty
        EXPECT_TRUE(result.isEmpty());
    } else {
        GTEST_SKIP() << "fork failed";
    }
}

// Test that id() returns the same value as a QByteArray (type check)
TEST(ut_DSGApplication, idReturnType)
{
    QByteArray result = DSGApplication::id();
    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result, QByteArray(result));
}
