// SPDX-FileCopyrightText: 2017 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dtkcore_global.h"
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSemaphore>
#include <QThread>
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
#include "dtimedloop.h"
DCORE_USE_NAMESPACE
#endif

#include "global/dconfig.h"

#include <memory>

// DConfig runs its async initialisation on a process-global worker thread and
// releases the per-config objects with deleteLater(). Both the posting and the
// deletion need an event loop turn, so unless that queued work is drained the
// objects are still alive when LeakSanitizer does its exit-time check and get
// reported as leaks. Call this from the TearDown of fixtures that create a
// DConfig.
inline void drainDConfigWorker()
{
    if (QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread()) {
        if (thread->isRunning()) {
            // The probe has to live in the worker thread for the queued call to
            // land there. The semaphore is shared rather than captured by
            // reference: a test that never used DConfig lets the wait below time
            // out with the call still pending, and it must stay valid until that
            // call is run (Qt drops posted events for a receiver that is gone).
            QObject probe;
            probe.moveToThread(thread);
            auto drained = std::make_shared<QSemaphore>();
            QMetaObject::invokeMethod(&probe, [drained] { drained->release(); },
                                      Qt::QueuedConnection);
            // Bounded: the probe is queued behind whatever init is pending, so
            // a test that never used DConfig just pays this and moves on.
            drained->tryAcquire(1, 200);
            probe.moveToThread(QThread::currentThread());
        }
    }
    // Run the deleteLater() calls the worker posted. Delivering only the
    // deferred deletes keeps this from pumping unrelated pending events.
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

// 有返回值的 lambda 表达式、函数里面使用 ASSERT_XXX
// 总之，不管有没有返回值，用它就对了
#ifndef HAVE_FUN
#define HAVE_FUN(X) [&](){X;}();
#endif

class ut_DUtil : public testing::Test
{
protected:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();
};
