// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QObject>
#include <gtest/gtest.h>
#include <QTest>
#include <QtConcurrent>

#include <DThreadUtils>

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
