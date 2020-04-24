/*
 * Copyright (C) 2016 ~ 2017 Deepin Technology Co., Ltd.
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
