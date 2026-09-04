// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <ddbusextendedabstractinterface.h>
#include "fakedbus/fakedbusservice.h"

#include <QTest>
#include <QSignalSpy>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>

DCORE_USE_NAMESPACE

class DBusExtendedInterfaceFoo : public DDBusExtendedAbstractInterface {
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
        { return "org.deepin.FakeDBusService"; }

    explicit DBusExtendedInterfaceFoo(QObject *parent = nullptr)
        : DDBusExtendedAbstractInterface(FakeDBusService::get_service(),
                                         FakeDBusService::get_path(),
                                         staticInterfaceName(),
                                         QDBusConnection::sessionBus(),
                                         parent) {
        connect(this, &DBusExtendedInterfaceFoo::propertyChanged, this,
                [this](const QString &propName, const QVariant &value){
            if (propName == QStringLiteral("strProperty"))
               {
                   const QString &strProperty = qvariant_cast<QString>(value);
                   if (m_strProp != strProperty)
                   {
                       m_strProp = strProperty;
                       Q_EMIT StrPropertyChanged(m_strProp);
                   }
                   return;
               }

               qWarning() << "property not handle: " << propName;
               return;
        });
    }
    ~DBusExtendedInterfaceFoo(){

    }

    Q_PROPERTY(QString strProperty READ strProperty WRITE setStrProperty NOTIFY StrPropertyChanged)
    QString strProperty() {
        return qvariant_cast<QString>(internalPropGet("strProperty", &m_strProp));
    }
    void setStrProperty(const QString &value) {
        internalPropSet("strProperty", QVariant::fromValue(value), &m_strProp);
    }
Q_SIGNALS: // SIGNALS
    void StrPropertyChanged(const QString & value) const;
private:
    QString m_strProp;
};

class ut_DDBusExtendedAbstractInterface : public testing::Test
{
public:
    void SetUp() override
    {
        if (!QDBusConnection::sessionBus().isConnected()) {
            GTEST_SKIP() << "Session bus not available";
        }
        m_fakeService = new FakeDBusService;
        m_dbusExtend = new DBusExtendedInterfaceFoo;
    }
    void TearDown() override
    {
        delete m_dbusExtend;
        delete m_fakeService;
    }
    DBusExtendedInterfaceFoo *m_dbusExtend = nullptr;
    FakeDBusService *m_fakeService = nullptr;
};

TEST_F(ut_DDBusExtendedAbstractInterface, sync)
{
    EXPECT_TRUE(m_dbusExtend->sync());
    m_dbusExtend->setSync(false, false);
    EXPECT_FALSE(m_dbusExtend->sync());
}

TEST_F(ut_DDBusExtendedAbstractInterface, useCache)
{
    EXPECT_FALSE(m_dbusExtend->useCache());
    m_dbusExtend->setUseCache(true);
    EXPECT_TRUE(m_dbusExtend->useCache());
}

TEST_F(ut_DDBusExtendedAbstractInterface, startServiceProcess)
{
    EXPECT_TRUE(m_dbusExtend->isValid());
    m_dbusExtend->startServiceProcess();
}

TEST_F(ut_DDBusExtendedAbstractInterface, getAllProperties)
{
    QSignalSpy spy(m_dbusExtend,  &DDBusExtendedAbstractInterface::asyncGetAllPropertiesFinished);
    m_dbusExtend->setSync(false);

    m_dbusExtend->getAllProperties();

    EXPECT_TRUE(QTest::qWaitFor([&spy, this]() {
        return spy.count() >= 1 && !m_dbusExtend->lastExtendedError().isValid();
    }, 2000));
}

// Covers getAllProperties() in sync mode — the reply parsing + onPropertiesChanged path
TEST_F(ut_DDBusExtendedAbstractInterface, getAllPropertiesSync)
{
    m_dbusExtend->setSync(true);
    m_dbusExtend->getAllProperties();
    // In sync mode the call completes synchronously; no error expected
    EXPECT_FALSE(m_dbusExtend->lastExtendedError().isValid());
}

// Covers getAllProperties() async when a call is already pending — early return
TEST_F(ut_DDBusExtendedAbstractInterface, getAllPropertiesAsyncDuplicatePending)
{
    m_dbusExtend->setSync(false);
    m_dbusExtend->getAllProperties();   // first async call
    m_dbusExtend->getAllProperties();   // second — should be a no-op (pending)
    QTest::qWait(500);
    SUCCEED();
}

TEST_F(ut_DDBusExtendedAbstractInterface, internalPropGet)
{
    QSignalSpy asyncPropertyFinishedSpy(m_dbusExtend,  &DDBusExtendedAbstractInterface::asyncPropertyFinished);
    QSignalSpy propertyChangedSpy(m_dbusExtend,  &DDBusExtendedAbstractInterface::propertyChanged);
    m_dbusExtend->setSync(false);

    // internalPropGet
    m_dbusExtend->strProperty();

    QObject::connect(m_dbusExtend,  &DDBusExtendedAbstractInterface::propertyChanged,
                     m_dbusExtend, [](const QString &propertyName, const QVariant &value){
        if (propertyName == "strProp")
            EXPECT_EQ(value.toString(), "testDBusService");

        qInfo() << "propertyChanged" << propertyName << value;
    });

    EXPECT_TRUE(QTest::qWaitFor([&]() {
        return asyncPropertyFinishedSpy.count() >= 1 &&
                propertyChangedSpy.count() >= 1 &&
                !m_dbusExtend->lastExtendedError().isValid();
    }, 2000));
}

TEST_F(ut_DDBusExtendedAbstractInterface, internalPropSet)
{
    QSignalSpy asyncPropertyFinishedSpy(m_dbusExtend,  &DDBusExtendedAbstractInterface::asyncSetPropertyFinished);
    QSignalSpy propertyChangedSpy(m_dbusExtend,  &DDBusExtendedAbstractInterface::propertyChanged);
    m_dbusExtend->setSync(false);

    // internalPropSet
    m_dbusExtend->setStrProperty("MyProp");

    QObject::connect(m_dbusExtend,  &DDBusExtendedAbstractInterface::propertyChanged,
                     m_dbusExtend, [](const QString &propertyName, const QVariant &value){
        if (propertyName == "strProp")
            EXPECT_EQ(value.toString(), "MyProp");

        qInfo() << "propertyChanged" << propertyName << value;
    });

    EXPECT_TRUE(QTest::qWaitFor([&]() {
        return asyncPropertyFinishedSpy.count() >= 1 &&
                propertyChangedSpy.count() >= 1 &&
                !m_dbusExtend->lastExtendedError().isValid();
    }, 2000));
}

// Covers internalPropGet with sync=true — calls property() directly
TEST_F(ut_DDBusExtendedAbstractInterface, internalPropGetSync)
{
    m_dbusExtend->setSync(true);
    QString val = m_dbusExtend->strProperty();
    EXPECT_EQ(val, m_fakeService->strproperty());
}

// Covers internalPropSet with sync=true — calls setProperty() directly
TEST_F(ut_DDBusExtendedAbstractInterface, internalPropSetSync)
{
    m_dbusExtend->setSync(true);
    m_dbusExtend->setStrProperty("syncValue");
    EXPECT_EQ(m_fakeService->strproperty(), QStringLiteral("syncValue"));
}

// Covers internalPropGet with useCache=true — returns cached value without DBus call
TEST_F(ut_DDBusExtendedAbstractInterface, internalPropGetUseCache)
{
    // First populate the cache via sync call
    m_dbusExtend->setSync(true);
    m_dbusExtend->strProperty();
    // Now enable cache and read — should return cached value
    m_dbusExtend->setUseCache(true);
    QString val = m_dbusExtend->strProperty();
    EXPECT_EQ(val, m_fakeService->strproperty());
    m_dbusExtend->setUseCache(false);
}

// Covers internalPropGet async with unknown property name (propertyIndex == -1)
TEST_F(ut_DDBusExtendedAbstractInterface, internalPropGetUnknownProperty)
{
    m_dbusExtend->setSync(false);
    QString dummy;
    QVariant result = m_dbusExtend->internalPropGet("nonExistentProp", &dummy);
    EXPECT_TRUE(m_dbusExtend->lastExtendedError().isValid());
    EXPECT_FALSE(result.isValid());
}

// Covers internalPropSet async with unknown property name (propertyIndex == -1)
TEST_F(ut_DDBusExtendedAbstractInterface, internalPropSetUnknownProperty)
{
    m_dbusExtend->setSync(false);
    QString dummy;
    m_dbusExtend->internalPropSet("nonExistentProp", QVariant("test"), &dummy);
    EXPECT_TRUE(m_dbusExtend->lastExtendedError().isValid());
}

// Covers onPropertiesChanged with unknown changed property and invalidated properties
TEST_F(ut_DDBusExtendedAbstractInterface, onPropertiesChangedUnknownAndInvalidated)
{
    // -fno-access-control allows calling private slots directly
    QSignalSpy invalidatedSpy(m_dbusExtend, &DDBusExtendedAbstractInterface::propertyInvalidated);

    QVariantMap changed;
    changed.insert("unknownChangedProp", QVariant(QStringLiteral("val")));
    QStringList invalidated;
    invalidated << "strProperty" << "unknownInvalidatedProp";

    m_dbusExtend->onPropertiesChanged(FakeDBusService::get_interface(), changed, invalidated);

    // strProperty is a known invalidated property → should emit propertyInvalidated
    EXPECT_GE(invalidatedSpy.count(), 1);
}

// Covers onPropertiesChanged with matching interface and known changed property
TEST_F(ut_DDBusExtendedAbstractInterface, onPropertiesChangedKnownProperty)
{
    QSignalSpy changedSpy(m_dbusExtend, &DDBusExtendedAbstractInterface::propertyChanged);

    QVariantMap changed;
    changed.insert("strProperty", QVariant(QStringLiteral("changedViaSignal")));

    m_dbusExtend->onPropertiesChanged(FakeDBusService::get_interface(), changed, QStringList());

    EXPECT_GE(changedSpy.count(), 1);
}

// Covers onPropertiesChanged with non-matching interface (should be a no-op)
TEST_F(ut_DDBusExtendedAbstractInterface, onPropertiesChangedWrongInterface)
{
    QSignalSpy changedSpy(m_dbusExtend, &DDBusExtendedAbstractInterface::propertyChanged);

    QVariantMap changed;
    changed.insert("strProperty", QVariant(QStringLiteral("shouldNotFire")));

    m_dbusExtend->onPropertiesChanged("org.deepin.SomeOtherInterface", changed, QStringList());

    EXPECT_EQ(changedSpy.count(), 0);
}

// Covers onDBusNameOwnerChanged — service appears
TEST_F(ut_DDBusExtendedAbstractInterface, onDBusNameOwnerChangedServiceAppears)
{
    QSignalSpy validSpy(m_dbusExtend, &DDBusExtendedAbstractInterface::serviceValidChanged);
    m_dbusExtend->onDBusNameOwnerChanged(FakeDBusService::get_service(), QString(), QStringLiteral("newOwner"));
    EXPECT_GE(validSpy.count(), 1);
}

// Covers onDBusNameOwnerChanged — service disappears
TEST_F(ut_DDBusExtendedAbstractInterface, onDBusNameOwnerChangedServiceDisappears)
{
    // First set the owner so the disappear branch can fire.
    // The appear branch checks: name == service() && oldOwner.isEmpty()
    // → sets m_dbusOwner = newOwner
    m_dbusExtend->onDBusNameOwnerChanged(FakeDBusService::get_service(), QString(), QStringLiteral("someOwner"));
    QSignalSpy validSpy(m_dbusExtend, &DDBusExtendedAbstractInterface::serviceValidChanged);
    // The disappear branch checks: name == m_dbusOwner && newOwner.isEmpty()
    // m_dbusOwner is now "someOwner", so name must be "someOwner" (not service()).
    // The signal is emitted synchronously — no qWait needed.
    m_dbusExtend->onDBusNameOwnerChanged(QStringLiteral("someOwner"), QStringLiteral("someOwner"), QString());
    EXPECT_GE(validSpy.count(), 1);
}

// Covers connectNotify for propertyChanged signal
TEST_F(ut_DDBusExtendedAbstractInterface, connectNotifyPropertyChanged)
{
    // Connecting to propertyChanged triggers connectNotify internally
    QObject obj;
    QObject::connect(m_dbusExtend, &DDBusExtendedAbstractInterface::propertyChanged, &obj,
                     [](const QString &, const QVariant &){});
    // No crash expected; the internal m_propertiesChangedConnected flag should be set
    SUCCEED();
}

// Covers disconnectNotify for propertyChanged signal
TEST_F(ut_DDBusExtendedAbstractInterface, disconnectNotifyPropertyChanged)
{
    QObject obj;
    QObject::connect(m_dbusExtend, &DDBusExtendedAbstractInterface::propertyChanged, &obj,
                     [](const QString &, const QVariant &){});
    QObject::disconnect(m_dbusExtend, &DDBusExtendedAbstractInterface::propertyChanged, &obj, nullptr);
    // No crash expected
    SUCCEED();
}

// Covers lastExtendedError accessor
TEST_F(ut_DDBusExtendedAbstractInterface, lastExtendedError)
{
    // Initially no error
    EXPECT_FALSE(m_dbusExtend->lastExtendedError().isValid());
}

#include "ut_ddbusextendedabstractinterface.moc"
