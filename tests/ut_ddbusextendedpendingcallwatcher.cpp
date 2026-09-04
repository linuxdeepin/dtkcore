// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "ddbusextendedpendingcallwatcher_p.h"

#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingCall>

DCORE_USE_NAMESPACE

class ut_DDBusExtendedPendingCallWatcher : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_sessionBusConnected = QDBusConnection::sessionBus().isConnected();
    }
    virtual void TearDown() override {}

    bool m_sessionBusConnected = false;
};

TEST_F(ut_DDBusExtendedPendingCallWatcher, inheritsQDBusPendingCallWatcher)
{
    if (!m_sessionBusConnected) {
        GTEST_SKIP() << "Session bus not available";
    }

    QDBusInterface iface(QStringLiteral("org.freedesktop.DBus"),
                         QStringLiteral("/org/freedesktop/DBus"),
                         QStringLiteral("org.freedesktop.DBus"));
    QDBusPendingCall call = iface.asyncCall(QStringLiteral("ListNames"));

    DDBusExtendedPendingCallWatcher watcher(call, QStringLiteral("testProp"), QVariant(QString("prev")));
    EXPECT_TRUE(watcher.inherits("QDBusPendingCallWatcher"));
}

TEST_F(ut_DDBusExtendedPendingCallWatcher, asyncProperty)
{
    if (!m_sessionBusConnected) {
        GTEST_SKIP() << "Session bus not available";
    }

    QDBusInterface iface(QStringLiteral("org.freedesktop.DBus"),
                         QStringLiteral("/org/freedesktop/DBus"),
                         QStringLiteral("org.freedesktop.DBus"));
    QDBusPendingCall call = iface.asyncCall(QStringLiteral("ListNames"));

    QString propName = QStringLiteral("myAsyncProperty");
    DDBusExtendedPendingCallWatcher watcher(call, propName, QVariant(QString("prev")));
    EXPECT_EQ(watcher.asyncProperty(), propName);
}

TEST_F(ut_DDBusExtendedPendingCallWatcher, previousValue)
{
    if (!m_sessionBusConnected) {
        GTEST_SKIP() << "Session bus not available";
    }

    QDBusInterface iface(QStringLiteral("org.freedesktop.DBus"),
                         QStringLiteral("/org/freedesktop/DBus"),
                         QStringLiteral("org.freedesktop.DBus"));
    QDBusPendingCall call = iface.asyncCall(QStringLiteral("ListNames"));

    QVariant prevVal = QVariant(42);
    DDBusExtendedPendingCallWatcher watcher(call, QStringLiteral("prop"), prevVal);
    EXPECT_EQ(watcher.previousValue(), prevVal);
}

TEST_F(ut_DDBusExtendedPendingCallWatcher, emptyAsyncProperty)
{
    if (!m_sessionBusConnected) {
        GTEST_SKIP() << "Session bus not available";
    }

    QDBusInterface iface(QStringLiteral("org.freedesktop.DBus"),
                         QStringLiteral("/org/freedesktop/DBus"),
                         QStringLiteral("org.freedesktop.DBus"));
    QDBusPendingCall call = iface.asyncCall(QStringLiteral("ListNames"));

    DDBusExtendedPendingCallWatcher watcher(call, QString(), QVariant());
    EXPECT_TRUE(watcher.asyncProperty().isEmpty());
    EXPECT_FALSE(watcher.previousValue().isValid());
}

TEST_F(ut_DDBusExtendedPendingCallWatcher, emptyPreviousValue)
{
    if (!m_sessionBusConnected) {
        GTEST_SKIP() << "Session bus not available";
    }

    QDBusInterface iface(QStringLiteral("org.freedesktop.DBus"),
                         QStringLiteral("/org/freedesktop/DBus"),
                         QStringLiteral("org.freedesktop.DBus"));
    QDBusPendingCall call = iface.asyncCall(QStringLiteral("ListNames"));

    DDBusExtendedPendingCallWatcher watcher(call, QStringLiteral("prop"), QVariant());
    EXPECT_FALSE(watcher.previousValue().isValid());
}

TEST_F(ut_DDBusExtendedPendingCallWatcher, destructor)
{
    if (!m_sessionBusConnected) {
        GTEST_SKIP() << "Session bus not available";
    }

    QDBusInterface iface(QStringLiteral("org.freedesktop.DBus"),
                         QStringLiteral("/org/freedesktop/DBus"),
                         QStringLiteral("org.freedesktop.DBus"));
    QDBusPendingCall call = iface.asyncCall(QStringLiteral("ListNames"));

    {
        DDBusExtendedPendingCallWatcher watcher(call, QStringLiteral("prop"), QVariant(1));
        EXPECT_EQ(watcher.asyncProperty(), QStringLiteral("prop"));
    }
    SUCCEED();
}
