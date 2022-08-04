// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QTimer>
#include <QThread>
#include <QMetaObject>
#include <QCoreApplication>

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
    memset(p, 0, size);
}

template <class T>
void SecureErase(T &obj)
{
    for (typename T::iterator i = obj.begin(); i != obj.end(); ++i) {
        *i = 0;
    }
    obj.clear();
}

}
