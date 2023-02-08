// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ut_singleton.h"

Singleton::Singleton(QObject *parent)
    : QObject(parent),
      count(0)
{
}

MultiSingletonTester::MultiSingletonTester(QObject *parent)
    : QObject(parent)
{
}

void MultiSingletonTester::run()
{
    Singleton::instance()->count.ref();
}

int MultiSingletonTester::count() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return Singleton::instance()->count.loadRelaxed();
#else
    return Singleton::instance()->count.load();
#endif
}
