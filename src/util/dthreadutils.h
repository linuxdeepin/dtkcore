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
#ifndef DTHREADUTILS_H
#define DTHREADUTILS_H

#include <dtkcore_global.h>
#include <QObject>
#include <QSemaphore>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

DCORE_BEGIN_NAMESPACE

namespace DThreadUtil {
typedef std::function<void()> FunctionType;

class FunctionCallProxy : public QObject
{
    Q_OBJECT
public:
    explicit FunctionCallProxy(QThread *thread);

Q_SIGNALS:
    void callInLiveThread(FunctionType *func);
};

template <typename ReturnType>
class _TMP
{
public:
    template <typename Fun, typename... Args>
    static ReturnType runInThread(QSemaphore *s, QThread *thread, Fun fun, Args&&... args)
    {
        if (QThread::currentThread() == thread)
            return fun(std::forward<Args>(args)...);

        ReturnType result;
        FunctionType proxyFun = [&] () {
            result = fun(std::forward<Args>(args)...);
            s->release();
        };

        FunctionCallProxy proxy(thread);
        proxy.moveToThread(thread);

        if (thread->loopLevel() <= 0) {
            qCritical() << Q_FUNC_INFO << thread << ", the thread no event loop";
        }

        proxy.callInLiveThread(&proxyFun);
        s->acquire();

        return result;
    }
};
template <>
class _TMP<void>
{
public:
    template <typename Fun, typename... Args>
    static void runInThread(QSemaphore *s, QThread *thread, Fun fun, Args&&... args)
    {
        if (QThread::currentThread() == thread)
            return fun(std::forward<Args>(args)...);

        FunctionType proxyFun = [&] () {
            fun(std::forward<Args>(args)...);
            s->release();
        };

        FunctionCallProxy proxy(thread);
        proxy.moveToThread(thread);

        if (thread->loopLevel() <= 0) {
            qCritical() << Q_FUNC_INFO << thread << ", the thread no event loop";
        }

        proxy.callInLiveThread(&proxyFun);
        s->acquire();
    }
};

template <typename Fun, typename... Args>
auto runInThread(QSemaphore *s, QThread *thread, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return _TMP<decltype(fun(args...))>::runInThread(s, thread, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
typename QtPrivate::FunctionPointer<Fun>::ReturnType runInThread(QSemaphore *s, QThread *thread, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return _TMP<typename QtPrivate::FunctionPointer<Fun>::ReturnType>::runInThread(s, thread, [&] {
        return (obj->*fun)(std::forward<Args>(args)...);
    });
}

template <typename Fun, typename... Args>
auto runInThread(QThread *thread, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    QSemaphore s;

    return runInThread(&s, thread, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
typename QtPrivate::FunctionPointer<Fun>::ReturnType runInThread(QThread *thread, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    QSemaphore s;

    return runInThread(&s, thread, obj, fun, std::forward<Args>(args)...);
}

template <typename Fun, typename... Args>
auto runInMainThread(Fun fun, Args&&... args) -> decltype(fun(args...))
{
    if (!QCoreApplication::instance()) {
        return fun(std::forward<Args>(args)...);
    }

    return runInThread(QCoreApplication::instance()->thread(), fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
typename QtPrivate::FunctionPointer<Fun>::ReturnType runInMainThread(typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    if (!QCoreApplication::instance()) {
        return (obj->*fun)(std::forward<Args>(args)...);
    }

    return runInThread(QCoreApplication::instance()->thread(), obj, fun, std::forward<Args>(args)...);
}
}

DCORE_END_NAMESPACE

#endif // DTHREADUTILS_H
