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

#pragma once

#include <QObject>

#include "base/dsingleton.h"

class Singleton : public QObject
    , public Dtk::Core::DSingleton<Singleton>
{
    Q_OBJECT
    friend class Dtk::Core::DSingleton<Singleton>;

public:
    explicit Singleton(QObject *parent = nullptr);

    QAtomicInt count;
};

class MultiSingletonTester : public QObject
{
    Q_OBJECT
public:
    explicit MultiSingletonTester(QObject *parent = nullptr);

    int count() const;

public Q_SLOTS:
    void run();
};
