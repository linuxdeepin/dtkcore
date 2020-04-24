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

#include <QObject>
#include <QTest>
#include <QDebug>

#include <util/DVtableHook>

DCORE_USE_NAMESPACE

class tst_DVtableHook : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void objectFun2ObjectFun();
    void objectFun2Fun();

    void fun2ObjectFun();
    void fun2Fun();
};

namespace TestClass {
class A {
public:
    virtual bool test(int v) {
        qDebug() << Q_FUNC_INFO << this << v;

        return false;
    }

    virtual ~A() {

    }
};

class B {
public:
    bool test(int v) {
        qDebug() << Q_FUNC_INFO << v;

        return true;
    }
};
}

using namespace TestClass;

void tst_DVtableHook::objectFun2ObjectFun()
{
    A *a = new A();
    B *b = new B();

    QVERIFY(DVtableHook::overrideVfptrFun(a, &A::test, b, &B::test));
    QVERIFY(DVtableHook::hasVtable(a));
    QVERIFY(a->test(0));
    DVtableHook::resetVfptrFun(a, &A::test);
    QVERIFY(!a->test(0));
    delete a;
    QVERIFY(!DVtableHook::hasVtable(a));
}

static bool test(A *obj, int v)
{
    qDebug() << Q_FUNC_INFO << obj << v;
    return true;
}

void tst_DVtableHook::objectFun2Fun()
{
    A *a = new A();
    QVERIFY(DVtableHook::overrideVfptrFun(a, &A::test, &test));
    QVERIFY(a->test(1));
    DVtableHook::resetVtable(a);
    QVERIFY(!DVtableHook::hasVtable(a));
}

void tst_DVtableHook::fun2ObjectFun()
{
    B *b = new B();
    QVERIFY(DVtableHook::overrideVfptrFun(&A::test, b, &B::test));
    A *a = new A();
    QVERIFY(DVtableHook::getVtableOfObject(a) == DVtableHook::getVtableOfClass<A>());
    QVERIFY(a->test(2));
}

void tst_DVtableHook::fun2Fun()
{
    QVERIFY(DVtableHook::overrideVfptrFun(&A::test, &test));
    A *a = new A();
    QVERIFY(a->test(3));
}

QTEST_MAIN(tst_DVtableHook)

#include "tst_dvtablehook.moc"
