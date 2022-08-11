// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dthreadutils.h"

DCORE_BEGIN_NAMESPACE

namespace DThreadUtil {
FunctionCallProxy::FunctionCallProxy(QThread *thread)
{
    qRegisterMetaType<QPointer<QObject>>();

    connect(this, &FunctionCallProxy::callInLiveThread, this, [] (QSemaphore *s, QPointer<QObject> target, FunctionType *func) {
        if (Q_LIKELY(target)) {
            (*func)();
        } else {
            qWarning() << "DThreadUtils::runInThread:" << "The target object is destoryed";
        }

        s->release();
    }, Qt::QueuedConnection);
    connect(thread, &QThread::finished, this, [this] {
        qWarning() << "DThreadUtils::runInThread:" << sender() << "the thread finished";
    }, Qt::DirectConnection);
}

void FunctionCallProxy::proxyCall(QSemaphore *s, QThread *thread, QObject *target, FunctionType fun)
{
    if (QThread::currentThread() == thread)
        return fun();

    FunctionCallProxy proxy(thread);
    proxy.moveToThread(thread);

    // 如果线程未开启事件循环，且不是主线程，则需要给出严重警告信息，因为可能会导致死锁
    if (thread->loopLevel() <= 0 && (!QCoreApplication::instance() || thread != QCoreApplication::instance()->thread())) {
        qCritical() << Q_FUNC_INFO << thread << ", the thread no event loop";
    }

    proxy.callInLiveThread(s, target ? target : &proxy, &fun);
    s->acquire();
}
} // end namespace DThreadUtil

DCORE_END_NAMESPACE
