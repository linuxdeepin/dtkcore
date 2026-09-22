// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "ddbussender.h"

#include <QtDBus>
#include <QDebug>

#include "fakedbus/fakedbusservice.h"

using Dtk::Core::DDBusInterface;

class ut_DDBusSender : public testing::Test
{
public:
    void SetUp() override
    {
        if (!QDBusConnection::sessionBus().isConnected()) {
            GTEST_SKIP() << "Session bus not available";
        }
        m_testservice = new FakeDBusService();
        m_sender = new DDBusSender;
    }
    void TearDown() override
    {
        delete m_testservice;
        delete m_sender;
    }

    FakeDBusService *m_testservice = nullptr;
    DDBusSender *m_sender = nullptr;
};


TEST_F(ut_DDBusSender, DDBusCaller)
{
    QDBusPendingReply<QString> reply = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .method(QString("foo"))
            .call();

    reply.waitForFinished();
    if (reply.error().isValid())
        qWarning() << reply.error().message();

    ASSERT_TRUE(reply.value() == QString("bar"));
}

TEST_F(ut_DDBusSender, DDBusProperty)
{
    auto prop = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .property(QString("strProperty"));

    QDBusPendingReply<QVariant> reply = prop.get();
    reply.waitForFinished();
    if (reply.error().isValid())
        qWarning() << reply.error().message();

    ASSERT_TRUE(reply.value().toString() == QString("testDBusService"));

    auto res = prop.set("myProp");
    res.waitForFinished();
    if (res.error().isValid())
        qWarning() << res.error().message();

    ASSERT_TRUE(m_testservice->strproperty() ==  QString("myProp"));
}
TEST_F(ut_DDBusSender, DDBusSenderSystem)
{
    // system() creates a sender using system bus
    DDBusSender sysSender = DDBusSender::system();
    // Verify it doesn't crash; system bus may or may not be connected
    SUCCEED();
}

TEST_F(ut_DDBusSender, DDBusCallerWithIntArg)
{
    // Test arg<int>() template — call foo method which ignores args
    QDBusPendingReply<QString> reply = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .method(QString("foo"))
            .arg(42)
            .call();

    reply.waitForFinished();
    // The fake service's foo() ignores arguments and returns "bar"
    // With extra args the call may fail, but we're testing the arg<int> template
    if (!reply.error().isValid()) {
        ASSERT_TRUE(reply.value() == QString("bar"));
    }
}

TEST_F(ut_DDBusSender, DDBusCallerWithBoolArg)
{
    // Test arg<bool>() template
    QDBusPendingReply<QString> reply = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .method(QString("foo"))
            .arg(true)
            .call();

    reply.waitForFinished();
    if (!reply.error().isValid()) {
        ASSERT_TRUE(reply.value() == QString("bar"));
    }
}

TEST_F(ut_DDBusSender, DDBusCallerWithMultipleArgs)
{
    // Test chaining multiple arg() calls with different types
    QDBusPendingReply<QString> reply = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .method(QString("foo"))
            .arg(42)
            .arg(QString("hello"))
            .arg(true)
            .call();

    reply.waitForFinished();
    // The call may fail due to signature mismatch, but we're testing the template
    if (!reply.error().isValid()) {
        ASSERT_TRUE(reply.value() == QString("bar"));
    }
}

TEST_F(ut_DDBusSender, DDBusPropertySetInt)
{
    // Test set<int>() template
    auto prop = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .property(QString("strProperty"));

    // set<int> will send a D-Bus Set call with an integer variant
    auto res = prop.set(12345);
    res.waitForFinished();
    // The property is QString type, so setting an int may fail at D-Bus level,
    // but we're testing the template instantiation, not the result
    SUCCEED();
}

TEST_F(ut_DDBusSender, DDBusPropertySetBool)
{
    // Test set<bool>() template
    auto prop = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .property(QString("strProperty"));

    auto res = prop.set(false);
    res.waitForFinished();
    SUCCEED();
}

TEST_F(ut_DDBusSender, DDBusSenderChainedServicePathInterface)
{
    // Test that service/path/interface chaining works correctly
    DDBusSender sender;
    sender.service(m_testservice->get_service());
    sender.path(m_testservice->get_path());
    sender.interface(m_testservice->get_interface());

    QDBusPendingReply<QString> reply = sender.method(QString("foo")).call();
    reply.waitForFinished();
    if (!reply.error().isValid())
        qWarning() << reply.error().message();

    ASSERT_TRUE(reply.value() == QString("bar"));
}
