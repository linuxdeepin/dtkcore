/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
