// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "base/dobject.h"
#include "base/private/dobject_p.h"

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

class TestObjectPrivate;
class TestObject : public DObject
{
    D_DECLARE_PRIVATE(TestObject)
public:
    TestObject();
    TestObject(TestObjectPrivate &dd, DObject *parent = nullptr);

    int value() const;
    void setValue(int v);

protected:
    TestObject(DObjectPrivate &dd, DObject *parent = nullptr);
};

class TestObjectPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(TestObject)
public:
    TestObjectPrivate(TestObject *qq);
    int data = 0;
};

TestObjectPrivate::TestObjectPrivate(TestObject *qq)
    : DObjectPrivate(qq)
{
}

TestObject::TestObject()
    : DObject(*new TestObjectPrivate(this))
{
}

TestObject::TestObject(TestObjectPrivate &dd, DObject *parent)
    : DObject(dd, parent)
{
}

TestObject::TestObject(DObjectPrivate &dd, DObject *parent)
    : DObject(dd, parent)
{
}

int TestObject::value() const
{
    D_DC(TestObject);
    return d->data;
}

void TestObject::setValue(int v)
{
    D_D(TestObject);
    d->data = v;
}

class ut_DObject : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ut_DObject, defaultConstructor)
{
    TestObject obj;
    EXPECT_EQ(obj.value(), 0);
}

TEST_F(ut_DObject, setValueGetValue)
{
    TestObject obj;
    obj.setValue(42);
    EXPECT_EQ(obj.value(), 42);
}

TEST_F(ut_DObject, setValueMultipleTimes)
{
    TestObject obj;
    obj.setValue(1);
    EXPECT_EQ(obj.value(), 1);
    obj.setValue(2);
    EXPECT_EQ(obj.value(), 2);
    obj.setValue(-100);
    EXPECT_EQ(obj.value(), -100);
}

TEST_F(ut_DObject, constructorWithPrivate)
{
    // DObject(DObjectPrivate &dd, ...) takes ownership of dd via QScopedPointer.
    // Heap-allocate the private so obj's destructor frees it without double-free.
    TestObjectPrivate *priv = new TestObjectPrivate(nullptr);
    TestObject obj(*priv, nullptr);
    obj.setValue(99);
    EXPECT_EQ(obj.value(), 99);
    // obj's destructor deletes priv via d_d_ptr
}

TEST_F(ut_DObject, dDPointerNotNull)
{
    TestObject obj;
    EXPECT_NE(obj.d_d_ptr.data(), nullptr);
}

TEST_F(ut_DObject, dDPointerOwnership)
{
    TestObject *obj = new TestObject();
    EXPECT_NE(obj->d_d_ptr.data(), nullptr);
    delete obj;
    SUCCEED();
}

TEST_F(ut_DObject, objectPrivateConstructor)
{
    TestObject *qq = nullptr;
    DObjectPrivate *priv = new TestObjectPrivate(nullptr);
    EXPECT_NE(priv, nullptr);
    delete priv;
}

TEST_F(ut_DObject, objectPrivateDestructor)
{
    DObjectPrivate *priv = new TestObjectPrivate(nullptr);
    delete priv;
    SUCCEED();
}

TEST_F(ut_DObject, qPointerInPrivate)
{
    TestObject obj;
    TestObjectPrivate *priv = reinterpret_cast<TestObjectPrivate *>(obj.d_d_ptr.data());
    EXPECT_EQ(priv->q_ptr, &obj);
}

TEST_F(ut_DObject, multipleObjectsIndependent)
{
    TestObject obj1;
    TestObject obj2;
    obj1.setValue(10);
    obj2.setValue(20);
    EXPECT_EQ(obj1.value(), 10);
    EXPECT_EQ(obj2.value(), 20);
}
