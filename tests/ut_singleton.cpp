/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
    return Singleton::instance()->count.load();
}
