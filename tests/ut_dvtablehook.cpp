/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <util/DVtableHook>

namespace TestClass {
class A
{
public:
    virtual bool test(int v);

    virtual ~A()
    {
    }
};
bool A::test(int v)
{
    qDebug() << Q_FUNC_INFO << this << v;

    return false;
}

class B
{
public:
    bool test(int v)
    {
        qDebug() << Q_FUNC_INFO << v;

        return true;
    }
};
} // namespace TestClass

class ut_DVtableHook : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        qDebug() << "*****************" << __FUNCTION__;
    }
    static void TearDownTestCase()
    {
        qDebug() << "*****************" << __FUNCTION__;
    }
    virtual void SetUp();
    virtual void TearDown();
};
void ut_DVtableHook::SetUp()
{
}
void ut_DVtableHook::TearDown()
{
}

using namespace TestClass;
DCORE_USE_NAMESPACE

static bool test(A *obj, int v)
{
    qDebug() << Q_FUNC_INFO << obj << v;
    return true;
}

TEST_F(ut_DVtableHook, objectFun2ObjectFun)
{
    A *a = new A();
    B *b = new B();

    ASSERT_TRUE(DVtableHook::overrideVfptrFun(a, &A::test, b, &B::test));
    ASSERT_TRUE(DVtableHook::hasVtable(a));
    ASSERT_TRUE(a->test(0));
    DVtableHook::resetVfptrFun(a, &A::test);
    ASSERT_TRUE(!a->test(0));
    delete a;
    ASSERT_TRUE(!DVtableHook::hasVtable(a));
}

TEST_F(ut_DVtableHook, objectFun2Fun)
{
    A *a = new A();
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(a, &A::test, &test));
    ASSERT_TRUE(a->test(1));
    DVtableHook::resetVtable(a);
    ASSERT_TRUE(!DVtableHook::hasVtable(a));
}

TEST_F(ut_DVtableHook, fun2ObjectFun)
{
    B *b = new B();
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(&A::test, b, &B::test));
    A *a = new A();
    ASSERT_TRUE(DVtableHook::getVtableOfObject(a) == DVtableHook::getVtableOfClass<A>());
    ASSERT_TRUE(a->test(4));
    delete a;
    delete b;
}

TEST_F(ut_DVtableHook, fun2Fun)
{
    ASSERT_TRUE(DVtableHook::overrideVfptrFun(&A::test, &test));
    A *a = new A();
    ASSERT_TRUE(a->test(5));
}
