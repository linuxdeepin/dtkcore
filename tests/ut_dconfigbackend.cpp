// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QVariant>
#include <QString>
#include <QStringList>
#include "global/dconfig.h"

DCORE_USE_NAMESPACE

// Minimal concrete subclass to test DConfigBackend default implementations
class TestConfigBackend : public DConfigBackend
{
public:
    bool isValid() const override { return true; }
    bool load(const QString &appId) override { Q_UNUSED(appId); return true; }
    QStringList keyList() const override { return QStringList{"key1", "key2"}; }
    QVariant value(const QString &key, const QVariant &fallback) const override
    {
        Q_UNUSED(key)
        return fallback;
    }
    void setValue(const QString &key, const QVariant &value) override
    {
        m_lastKey = key;
        m_lastValue = value;
    }

    QString m_lastKey;
    QVariant m_lastValue;
};

TEST(ut_DConfigBackend, defaultReset)
{
    TestConfigBackend backend;
    backend.reset("testKey");
    // reset() default implementation calls setValue(key, QVariant())
    EXPECT_EQ(backend.m_lastKey, QString("testKey"));
    EXPECT_EQ(backend.m_lastValue, QVariant());
}

TEST(ut_DConfigBackend, defaultName)
{
    TestConfigBackend backend;
    // name() default implementation returns empty string
    EXPECT_EQ(backend.name(), QString(""));
}

TEST(ut_DConfigBackend, defaultIsDefaultValue)
{
    TestConfigBackend backend;
    // isDefaultValue() default implementation returns true
    EXPECT_TRUE(backend.isDefaultValue("anyKey"));
}

TEST(ut_DConfigBackend, defaultIsReadOnly)
{
    TestConfigBackend backend;
    // isReadOnly() default implementation returns false
    EXPECT_FALSE(backend.isReadOnly("anyKey"));
}

TEST(ut_DConfigBackend, defaultIsReadOnlyMultipleKeys)
{
    TestConfigBackend backend;
    EXPECT_FALSE(backend.isReadOnly("key1"));
    EXPECT_FALSE(backend.isReadOnly("key2"));
    EXPECT_FALSE(backend.isReadOnly("nonexistent"));
}

TEST(ut_DConfigBackend, defaultIsDefaultValueMultipleKeys)
{
    TestConfigBackend backend;
    EXPECT_TRUE(backend.isDefaultValue("key1"));
    EXPECT_TRUE(backend.isDefaultValue("key2"));
    EXPECT_TRUE(backend.isDefaultValue("nonexistent"));
}

TEST(ut_DConfigBackend, concreteIsValid)
{
    TestConfigBackend backend;
    EXPECT_TRUE(backend.isValid());
}

TEST(ut_DConfigBackend, concreteLoad)
{
    TestConfigBackend backend;
    EXPECT_TRUE(backend.load("testApp"));
}

TEST(ut_DConfigBackend, concreteKeyList)
{
    TestConfigBackend backend;
    QStringList keys = backend.keyList();
    EXPECT_EQ(keys.size(), 2);
    EXPECT_TRUE(keys.contains("key1"));
    EXPECT_TRUE(keys.contains("key2"));
}

TEST(ut_DConfigBackend, concreteValueWithFallback)
{
    TestConfigBackend backend;
    QVariant fallback("defaultValue");
    QVariant result = backend.value("key1", fallback);
    EXPECT_EQ(result, fallback);
}

TEST(ut_DConfigBackend, concreteSetValue)
{
    TestConfigBackend backend;
    backend.setValue("key1", QVariant(42));
    EXPECT_EQ(backend.m_lastKey, QString("key1"));
    EXPECT_EQ(backend.m_lastValue, QVariant(42));
}

TEST(ut_DConfigBackend, concreteSetValueAndReset)
{
    TestConfigBackend backend;
    backend.setValue("key1", QVariant("hello"));
    EXPECT_EQ(backend.m_lastValue, QVariant("hello"));

    // reset should clear the value
    backend.reset("key1");
    EXPECT_EQ(backend.m_lastKey, QString("key1"));
    EXPECT_EQ(backend.m_lastValue, QVariant());
}

TEST(ut_DConfigBackend, virtualDestructor)
{
    // Verify virtual destructor works correctly through base pointer
    DConfigBackend *backend = new TestConfigBackend();
    delete backend;
    SUCCEED();
}
