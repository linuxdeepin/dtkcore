/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>

#include "base/dsingleton.h"

class Singleton : public QObject, public Dtk::Core::DSingleton<Singleton>
{
    Q_OBJECT
    friend class Dtk::Core::DSingleton<Singleton>;
public:
    explicit Singleton(QObject *parent = 0);

    void test();
};

class MultiSingletonTester : public QObject
{
    Q_OBJECT
public:
    explicit MultiSingletonTester(QObject *parent = 0);

    void run();
};


