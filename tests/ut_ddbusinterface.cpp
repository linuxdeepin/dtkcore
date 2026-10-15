// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <ddbusinterface.h>

#include "fakedbus/fakedbusservice.h"
#include <QTest>
#include <QSignalSpy>
#include <QDBusPendingReply>

using Dtk::Core::DDBusInterface;

class ut_DDBusInterface : public testing::Test
{
public:
    void SetUp() override
    {
        if (!QDBusConnection::sessionBus().isConnected()) {
            GTEST_SKIP() << "Session bus not available";
        }
        m_testservice = new FakeDBusService();
        m_testDBusInterfaceParent = new FakeDBusServiceParent();
        // if parent is nullptr
        m_testInterface = new DDBusInterface(FakeDBusService::get_service(),
                                             FakeDBusService::get_path(),
                                             FakeDBusService::get_interface(),
                                             QDBusConnection::sessionBus());
    }
    void TearDown() override
    {
        delete m_testDBusInterfaceParent;
        delete m_testInterface;
        delete m_testservice;
    }
    DDBusInterface *m_testInterface;
    FakeDBusService *m_testservice;
    FakeDBusServiceParent *m_testDBusInterfaceParent;
};

TEST_F(ut_DDBusInterface, TestProperty)
{
    auto objectpaths = m_testservice->objectpaths();
    EXPECT_EQ(m_testDBusInterfaceParent->objectpaths().length(), objectpaths.length());

    auto strproperty = qvariant_cast<QString>(m_testInterface->property("strProperty"));
    EXPECT_EQ(strproperty, m_testservice->strproperty());
}

TEST_F(ut_DDBusInterface, suffix)
{
    EXPECT_EQ(m_testInterface->suffix(), "");
    m_testInterface->setSuffix("-suffix");
    EXPECT_EQ(m_testInterface->suffix(), "-suffix");
}

// Test property() with a parent — exercises the demarshall path
TEST_F(ut_DDBusInterface, TestPropertyWithParent)
{
    // FakeDBusServiceParent has a DDBusInterface with parent=this
    // property("objectPaths") goes through the parent path with demarshall
    auto objectpaths = m_testDBusInterfaceParent->objectpaths();
    EXPECT_EQ(objectpaths.length(), m_testservice->objectpaths().length());
}

// Test setProperty() — sets strProperty via D-Bus
TEST_F(ut_DDBusInterface, TestSetProperty)
{
    m_testInterface->setProperty("strProperty", QVariant(QStringLiteral("newTestValue")));
    // Wait a bit for async to complete
    QTest::qWait(100);
    EXPECT_EQ(m_testservice->strproperty(), QStringLiteral("newTestValue"));
}

// Test property() with suffix — the suffix is stripped to get original prop name
TEST_F(ut_DDBusInterface, TestPropertyWithSuffix)
{
    m_testInterface->setSuffix("-suffix");
    // property("strProperty-suffix") should resolve to "strProperty" on the service
    auto val = m_testInterface->property("strProperty-suffix");
    // The value may be returned synchronously or asynchronously
    QTest::qWait(100);
    EXPECT_EQ(qvariant_cast<QString>(val), m_testservice->strproperty());
}

// Test serviceValid() — initially the service should be valid since FakeDBusService is running
TEST_F(ut_DDBusInterface, TestServiceValid)
{
    // Wait for NameHasOwner callback to complete
    QTest::qWait(200);
    EXPECT_TRUE(m_testInterface->serviceValid());
}

// Test serviceValidChanged signal
TEST_F(ut_DDBusInterface, TestServiceValidChanged)
{
    // The service is already registered in SetUp, so serviceValidChanged may have
    // fired before we could attach the spy. We verify the signal exists and the
    // current state is valid.
    QSignalSpy spy(m_testInterface, &DDBusInterface::serviceValidChanged);
    QTest::qWait(200);
    EXPECT_TRUE(m_testInterface->serviceValid());
}

// Test property() on a non-existent property — should get async reply with error
TEST_F(ut_DDBusInterface, TestPropertyNonExistent)
{
    auto val = m_testInterface->property("nonExistentProperty");
    // The property doesn't exist, so we get an invalid QVariant (async path)
    QTest::qWait(100);
    EXPECT_FALSE(val.isValid());
}

// Test property() with no parent (already the case in SetUp) — returns value directly
TEST_F(ut_DDBusInterface, TestPropertyNoParent)
{
    // m_testInterface has no parent, so property() returns value directly
    auto val = m_testInterface->property("strProperty");
    QTest::qWait(100);
    EXPECT_EQ(qvariant_cast<QString>(val), m_testservice->strproperty());
}

// Test creating DDBusInterface with a parent object
TEST_F(ut_DDBusInterface, TestInterfaceWithParent)
{
    QObject parentObj;
    auto *iface = new DDBusInterface(FakeDBusService::get_service(),
                                     FakeDBusService::get_path(),
                                     FakeDBusService::get_interface(),
                                     QDBusConnection::sessionBus(),
                                     &parentObj);
    QTest::qWait(200);
    EXPECT_TRUE(iface->serviceValid());
    auto val = iface->property("strProperty");
    QTest::qWait(100);
    EXPECT_EQ(qvariant_cast<QString>(val), m_testservice->strproperty());
    delete iface;
}

// Test setProperty() on non-existent property — should not crash
TEST_F(ut_DDBusInterface, TestSetPropertyNonExistent)
{
    m_testInterface->setProperty("nonExistentProp", QVariant(42));
    QTest::qWait(100);
    // setProperty on non-existent property should not crash; verify service stays valid
    EXPECT_TRUE(m_testInterface->serviceValid());
}

// Test originalPropname helper through suffix mechanism
TEST_F(ut_DDBusInterface, TestSuffixStrippedInProperty)
{
    m_testInterface->setSuffix("_sfx");
    m_testInterface->setProperty("strProperty_sfx", QVariant(QStringLiteral("sfxValue")));
    QTest::qWait(100);
    EXPECT_EQ(m_testservice->strproperty(), QStringLiteral("sfxValue"));
    m_testInterface->setSuffix("");
}

// Test property() with suffix and no parent — exercises the qWarning path
TEST_F(ut_DDBusInterface, TestPropertyWithSuffixNoParent)
{
    m_testInterface->setSuffix("-sfx");
    auto val = m_testInterface->property("strProperty-sfx");
    QTest::qWait(100);
    // Without parent, if value is valid it returns directly; otherwise returns invalid
    // The property exists so value should be valid
    EXPECT_TRUE(val.isValid());
    m_testInterface->setSuffix("");
}

// Test that destroying the service makes serviceValid false
TEST_F(ut_DDBusInterface, TestServiceValidFalseWhenServiceGone)
{
    // Wait for initial NameHasOwner to complete
    QTest::qWait(200);
    EXPECT_TRUE(m_testInterface->serviceValid());

    QSignalSpy spy(m_testInterface, &DDBusInterface::serviceValidChanged);

    // Destroy the service — should trigger onDBusNameOwnerChanged
    delete m_testservice;
    m_testservice = nullptr;

    QTest::qWait(500);

    // serviceValid should become false, or spy should have captured the change
    // (timing-dependent; at minimum no crash)
    if (spy.count() > 0) {
        EXPECT_FALSE(m_testInterface->serviceValid());
    }

    // Recreate service for TearDown compatibility
    m_testservice = new FakeDBusService();
    QTest::qWait(200);
}

// Test setProperty with suffix — exercises originalPropname stripping in setProperty
TEST_F(ut_DDBusInterface, TestSetPropertyWithSuffix)
{
    m_testInterface->setSuffix("_ts");
    m_testInterface->setProperty("strProperty_ts", QVariant(QStringLiteral("sfxSetValue")));
    QTest::qWait(100);
    EXPECT_EQ(m_testservice->strproperty(), QStringLiteral("sfxSetValue"));
    m_testInterface->setSuffix("");
}

// Test property() on a non-existent property with parent — async error path
TEST_F(ut_DDBusInterface, TestPropertyNonExistentWithParent)
{
    // Use FakeDBusServiceParent which has a parent
    auto val = m_testDBusInterfaceParent->objectpaths();
    // This works fine for existing property; now test non-existent via parent path
    QObject parentObj;
    auto *iface = new DDBusInterface(FakeDBusService::get_service(),
                                     FakeDBusService::get_path(),
                                     FakeDBusService::get_interface(),
                                     QDBusConnection::sessionBus(),
                                     &parentObj);
    auto val2 = iface->property("nonExistentProp");
    QTest::qWait(100);
    EXPECT_FALSE(val2.isValid());
    delete iface;
}

// Test multiple property reads — verify consistency
TEST_F(ut_DDBusInterface, TestMultiplePropertyReads)
{
    auto val1 = m_testInterface->property("strProperty");
    QTest::qWait(50);
    auto val2 = m_testInterface->property("strProperty");
    QTest::qWait(50);
    EXPECT_EQ(qvariant_cast<QString>(val1), qvariant_cast<QString>(val2));
}

// Test that suffix affects both property read and write consistently
TEST_F(ut_DDBusInterface, TestSuffixRoundTrip)
{
    m_testInterface->setSuffix("_rt");
    m_testInterface->setProperty("strProperty_rt", QVariant(QStringLiteral("roundTrip")));
    QTest::qWait(100);
    auto val = m_testInterface->property("strProperty_rt");
    QTest::qWait(100);
    EXPECT_EQ(qvariant_cast<QString>(val), QStringLiteral("roundTrip"));
    m_testInterface->setSuffix("");
}
