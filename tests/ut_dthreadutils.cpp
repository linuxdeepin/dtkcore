/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
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
#include <gtest/gtest.h>
#include <QTest>
#include <QtConcurrent>

#include <util/DThreadUtils>

DCORE_USE_NAMESPACE

class ThreadUtils : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void testCallInMainThread();
};

void ThreadUtils::testCallInMainThread()
{
    DThreadUtil::runInMainThread([]() {
        bool result = QThread::currentThread() == QCoreApplication::instance()->thread();
        ASSERT_TRUE(result);
    });

    auto fe = QtConcurrent::run([] {
        ASSERT_TRUE(DThreadUtil::runInMainThread([](QThread *thread) -> bool {
            return QThread::currentThread() == QCoreApplication::instance()->thread() && QThread::currentThread() != thread;
        }, QThread::currentThread()));
    });

    ASSERT_TRUE(QTest::qWaitFor([&] {
        return fe.isFinished();
    }));
}

class ut_DThreadUtils : public testing::Test
{
public:
    virtual void SetUp()
    {
        m_threadutil = new ThreadUtils();
    }
    virtual void TearDown()
    {
        delete m_threadutil;
    }

protected:
    ThreadUtils *m_threadutil = nullptr;
};

TEST_F(ut_DThreadUtils, CallInMainThread)
{
    ASSERT_TRUE(m_threadutil);
    m_threadutil->testCallInMainThread();
}

#include "ut_dthreadutils.moc"
