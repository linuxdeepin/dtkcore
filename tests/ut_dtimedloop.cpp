// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/dtimedloop.h"

#include <gtest/gtest.h>
#include <QTimer>
#include <QCoreApplication>

DCORE_USE_NAMESPACE

class ut_DTimedLoop : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(ut_DTimedLoop, defaultConstructor)
{
    DTimedLoop loop;
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, constructorWithParent)
{
    DTimedLoop loop(nullptr);
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, execWithDuration)
{
    DTimedLoop loop;
    loop.exec(10);
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, execWithDurationAndName)
{
    DTimedLoop loop;
    loop.exec(10, QStringLiteral("testExec"));
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, execWithName)
{
    DTimedLoop loop;
    loop.setTimeDump(true);
    QTimer::singleShot(0, &loop, [&loop]() { loop.exit(0); });
    loop.exec(QStringLiteral("namedExec"));
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, execWithFlags)
{
    DTimedLoop loop;
    QTimer::singleShot(0, &loop, [&loop]() { loop.exit(0); });
    loop.exec(QEventLoop::AllEvents);
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, exitDuringExec)
{
    DTimedLoop loop;
    QTimer::singleShot(0, &loop, [&loop]() { loop.exit(123); });
    int ret = loop.exec(10000);
    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, runningTimeAfterExec10)
{
    DTimedLoop loop;
    loop.exec(10);
    int rt = loop.runningTime();
    EXPECT_GE(rt, 8);
}

TEST_F(ut_DTimedLoop, runningTimeWhileRunning)
{
    DTimedLoop loop;
    QTimer::singleShot(50, &loop, [&loop]() { loop.exit(0); });
    loop.exec();
    int rt = loop.runningTime();
    EXPECT_GE(rt, 0);
}

TEST_F(ut_DTimedLoop, setTimeDumpTrue)
{
    DTimedLoop loop;
    loop.setTimeDump(true);
    loop.exec(10);
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, setTimeDumpFalse)
{
    DTimedLoop loop;
    loop.setTimeDump(false);
    loop.exec(10);
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, exitBeforeExec)
{
    DTimedLoop loop;
    loop.exit(42);
    loop.exec(10);
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, execNegativeDuration)
{
    DTimedLoop loop;
    loop.exec(-1);
    EXPECT_FALSE(loop.isRunning());
}

TEST_F(ut_DTimedLoop, execZeroDuration)
{
    DTimedLoop loop;
    loop.exec(0);
    EXPECT_FALSE(loop.isRunning());
}
