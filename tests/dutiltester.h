/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DUTILTESTER_H
#define DUTILTESTER_H

#include <QObject>

class TestDUtil: public QObject
{
    Q_OBJECT
public:
    TestDUtil();

private Q_SLOTS:
    void testLogPath();
    void testPathChange();
    void testDSingleton();
};

#endif // DUTILTESTER_H
