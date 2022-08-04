// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTHREADUTILS_H
#define DTHREADUTILS_H

#include <dtkcore_global.h>
#include <QObject>
#include <QSemaphore>
#include <QThread>
#include <QCoreApplication>
#include <QPointer>
#include <QDebug>

DCORE_BEGIN_NAMESPACE

namespace DThreadUtil {
typedef std::function<void()> FunctionType;

class LIBDTKCORESHARED_EXPORT FunctionCallProxy : public QObject
{
    Q_OBJECT
public:
    explicit FunctionCallProxy(QThread *thread);

    static void proxyCall(QSemaphore *s, QThread *thread, QObject *target, FunctionType fun);

Q_SIGNALS:
    void callInLiveThread(QSemaphore *s, QPointer<QObject> target, FunctionType *func);
};

template <typename ReturnType>
class LIBDTKCORESHARED_EXPORT _TMP
{
public:
    inline static ReturnType runInThread(QSemaphore *s, QThread *thread, QObject *target, std::function<ReturnType()> fun)
    {
        ReturnType result;
        FunctionType proxyFun = [&result, &fun] () {
            result = fun();
        };

        FunctionCallProxy::proxyCall(s, thread, target, proxyFun);
        return result;
    }

    template <typename T>
    inline static typename std::enable_if<!std::is_base_of<QObject, T>::value, ReturnType>::type
            runInThread(QSemaphore *s, QThread *thread, T *, std::function<ReturnType()> fun)
    {
        return runInThread(s, thread, static_cast<QObject*>(nullptr), fun);
    }
};
template <>
class LIBDTKCORESHARED_EXPORT _TMP<void>
{
public:
    inline static void runInThread(QSemaphore *s, QThread *thread, QObject *target, std::function<void()> fun)
    {
        FunctionCallProxy::proxyCall(s, thread, target, fun);
    }

    template <typename T>
    inline static typename std::enable_if<!std::is_base_of<QObject, T>::value, void>::type
            runInThread(QSemaphore *s, QThread *thread, T *, std::function<void()> fun)
    {
        return runInThread(s, thread, static_cast<QObject*>(nullptr), fun);
    }
};

template <typename Fun, typename... Args>
inline auto runInThread(QSemaphore *s, QThread *thread, QObject *target, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return _TMP<decltype(fun(args...))>::runInThread(s, thread, target, std::bind(fun, std::forward<Args>(args)...));
}
template <typename Fun, typename... Args>
inline auto runInThread(QSemaphore *s, QThread *thread, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return runInThread(s, thread, nullptr, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInThread(QSemaphore *s, QThread *thread, QObject *target, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return _TMP<typename QtPrivate::FunctionPointer<Fun>::ReturnType>::runInThread(s, thread, target, std::bind(fun, obj, std::forward<Args>(args)...));
}
template <typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInThread(QSemaphore *s, QThread *thread, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return _TMP<typename QtPrivate::FunctionPointer<Fun>::ReturnType>::runInThread(s, thread, obj, std::bind(fun, obj, std::forward<Args>(args)...));
}

template <typename Fun, typename... Args>
inline auto runInThread(QThread *thread, QObject *target, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    QSemaphore s;

    return runInThread(&s, thread, target, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
inline auto runInThread(QThread *thread, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return runInThread(thread, nullptr, fun, std::forward<Args>(args)...);
}
template <typename T, typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInThread(QThread *thread, T *target, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    QSemaphore s;

    return runInThread(&s, thread, target, obj, fun, std::forward<Args>(args)...);
}

template <typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInThread(QThread *thread, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return runInThread(thread, obj, obj, fun, std::forward<Args>(args)...);
}

template <typename Fun, typename... Args>
inline auto runInMainThread(QObject *target, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    if (!QCoreApplication::instance()) {
        return fun(std::forward<Args>(args)...);
    }

    return runInThread(QCoreApplication::instance()->thread(), target, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
inline auto runInMainThread(Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return runInMainThread(nullptr, fun, std::forward<Args>(args)...);
}

template <typename T, typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInMainThread(T *target, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    if (!QCoreApplication::instance()) {
        return (obj->*fun)(std::forward<Args>(args)...);
    }

    return runInThread(QCoreApplication::instance()->thread(), target, obj, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInMainThread(typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return runInMainThread(obj, obj, fun, std::forward<Args>(args)...);
}
}

DCORE_END_NAMESPACE

#endif // DTHREADUTILS_H
