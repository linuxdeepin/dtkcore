// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>

namespace TestClass {
class A
{
public:
    virtual bool test(int v);

    virtual ~A() {}
};

class B
{
public:
    virtual bool test(int v)
    {
        qDebug() << Q_FUNC_INFO << v;

        return true;
    }

    virtual ~B() {}
};
}  // namespace TestClass
