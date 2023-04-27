// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QTimer>
#include <QThread>
#include <QMetaObject>
#include <QCoreApplication>
#include <type_traits>
#include <cstring>

namespace DUtil
{

template <typename Func1>
inline void TimerSingleShot(int msec,  Func1 slot)
{
#if QT_VERSION >= 0x050500
    QTimer::singleShot(msec, slot);
#else
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    timer->setInterval(msec);
    timer->moveToThread(qApp->thread());
    QObject::connect(timer, &QTimer::timeout, slot);
    QObject::connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
    if (QThread::currentThread() == qApp->thread()) { timer->start(); }
    else { QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection); }
#endif
}

template <class T>
void SecureErase(T *p, size_t size)
{
    static_assert(std::is_standard_layout<T>::value && std::is_trivially_destructible<T>::value,
                  "try to erase content of raw pointer, but type T isn't suitable");

    std::memset(p, 0, size);
}

template <class T>
void SecureErase(T &obj)
{
    static_assert(std::is_default_constructible<typename T::value_type>::value,
                  "container's value type must have a default constructor.");

    for (typename T::iterator i = obj.begin(); i != obj.end(); ++i) {
        *i = typename T::value_type{};
    }
}

}
