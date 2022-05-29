/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Intel Corporation.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qplatformdefs.h"

#include "qcoreapplication.h"
#include "qpair.h"
#include "qsocketnotifier.h"
#include "qthread.h"
#include "qelapsedtimer.h"

#include <QtCore/qlist.h>
#include "private/qcore_unix_p.h"
#include <QtCore/qvarlengtharray.h>
#include "private/qtimerinfo_unix_p.h"

#include "qeventdispatcher_epoll.h"
#include <private/qthread_p.h>
#include <private/qcoreapplication_p.h>
#include <private/qcore_unix_p.h>
#include <private/qabstracteventdispatcher_p.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef QT_NO_EVENTFD
#  include <sys/eventfd.h>
#endif

// VxWorks doesn't correctly set the _POSIX_... options
#if defined(Q_OS_VXWORKS)
#  if defined(_POSIX_MONOTONIC_CLOCK) && (_POSIX_MONOTONIC_CLOCK <= 0)
#    undef _POSIX_MONOTONIC_CLOCK
#    define _POSIX_MONOTONIC_CLOCK 1
#  endif
#  include <pipeDrv.h>
#  include <sys/time.h>
#endif

#if (_POSIX_MONOTONIC_CLOCK-0 <= 0) || defined(QT_BOOTSTRAPPED)
#  include <sys/times.h>
#endif

QT_BEGIN_NAMESPACE

static const char *socketType(QSocketNotifier::Type type)
{
    switch (type) {
    case QSocketNotifier::Read:
        return "Read";
    case QSocketNotifier::Write:
        return "Write";
    case QSocketNotifier::Exception:
        return "Exception";
    }

    Q_UNREACHABLE();
}

class QSocketNotifier;
struct Q_CORE_EXPORT QSocketNotifierSetEpoll final
{
    inline QSocketNotifierSetEpoll() Q_DECL_NOTHROW;

    inline bool isEmpty() const Q_DECL_NOTHROW;
    inline short events() const Q_DECL_NOTHROW;

    QSocketNotifier *notifiers[3];
};

Q_DECLARE_TYPEINFO(QSocketNotifierSetEpoll, Q_PRIMITIVE_TYPE);

class Q_CORE_EXPORT QEventDispatcherEpollPrivate : public QAbstractEventDispatcherPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherEpoll)

public:
    QEventDispatcherEpollPrivate();
    ~QEventDispatcherEpollPrivate();

    int activateTimers();

    void markPendingSocketNotifiers();
    int activateSocketNotifiers();
    void setSocketNotifierPending(QSocketNotifier *notifier);

    int epfd = -1;

    QVector<epoll_event> pollfds;

    QHash<int, QSocketNotifierSetEpoll> socketNotifiers;
    QVector<QSocketNotifier *> pendingNotifiers;

    QTimerInfoList timerList;
    QAtomicInt interrupt; // bool

    struct QThreadPipe
    {
        QThreadPipe();
        ~QThreadPipe();

        bool init();
        epoll_event prepare() const;

        void wakeUp();
        int check(const epoll_event &pfd);

        // note for eventfd(7) support:
        // if fds[1] is -1, then eventfd(7) is in use and is stored in fds[0]
        int fds[2];
        QAtomicInt wakeUps;

    #if defined(Q_OS_VXWORKS)
        static const int len_name = 20;
        char name[len_name];
    #endif
    } threadPipe;
};

inline QSocketNotifierSetEpoll::QSocketNotifierSetEpoll() Q_DECL_NOTHROW
{
    notifiers[0] = nullptr;
    notifiers[1] = nullptr;
    notifiers[2] = nullptr;
}

inline bool QSocketNotifierSetEpoll::isEmpty() const Q_DECL_NOTHROW
{
    return !notifiers[0] && !notifiers[1] && !notifiers[2];
}

inline short QSocketNotifierSetEpoll::events() const Q_DECL_NOTHROW
{
    short result = 0;

    if (notifiers[0])
        result |= POLLIN;

    if (notifiers[1])
        result |= POLLOUT;

    if (notifiers[2])
        result |= POLLPRI;

    return result;
}

QEventDispatcherEpollPrivate::QThreadPipe::QThreadPipe()
{
    fds[0] = -1;
    fds[1] = -1;
#if defined(Q_OS_VXWORKS)
    name[0] = '\0';
#endif
}

QEventDispatcherEpollPrivate::QThreadPipe::~QThreadPipe()
{
    if (fds[0] >= 0)
        close(fds[0]);

    if (fds[1] >= 0)
        close(fds[1]);

#if defined(Q_OS_VXWORKS)
    pipeDevDelete(name, true);
#endif
}

#if defined(Q_OS_VXWORKS)
static void initThreadPipeFD(int fd)
{
    int ret = fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (ret == -1)
        perror("QEventDispatcherEpollPrivate: Unable to init thread pipe");

    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        perror("QEventDispatcherEpollPrivate: Unable to get flags on thread pipe");

    ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (ret == -1)
        perror("QEventDispatcherEpollPrivate: Unable to set flags on thread pipe");
}
#endif

bool QEventDispatcherEpollPrivate::QThreadPipe::init()
{
#if defined(Q_OS_NACL)
   // do nothing.
#elif defined(Q_OS_VXWORKS)
    qsnprintf(name, sizeof(name), "/pipe/qt_%08x", int(taskIdSelf()));

    // make sure there is no pipe with this name
    pipeDevDelete(name, true);

    // create the pipe
    if (pipeDevCreate(name, 128 /*maxMsg*/, 1 /*maxLength*/) != OK) {
        perror("QThreadPipe: Unable to create thread pipe device %s", name);
        return false;
    }

    if ((fds[0] = open(name, O_RDWR, 0)) < 0) {
        perror("QThreadPipe: Unable to open pipe device %s", name);
        return false;
    }

    initThreadPipeFD(fds[0]);
    fds[1] = fds[0];
#else
#  ifndef QT_NO_EVENTFD
    if ((fds[0] = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) >= 0)
        return true;
#  endif
    if (qt_safe_pipe(fds, O_NONBLOCK) == -1) {
        perror("QThreadPipe: Unable to create pipe");
        return false;
    }
#endif

    return true;
}

epoll_event QEventDispatcherEpollPrivate::QThreadPipe::prepare() const
{
    return qt_make_epollfd(fds[0], EPOLLIN);
}

void QEventDispatcherEpollPrivate::QThreadPipe::wakeUp()
{
    if (wakeUps.testAndSetAcquire(0, 1)) {
#ifndef QT_NO_EVENTFD
        if (fds[1] == -1) {
            // eventfd
            eventfd_t value = 1;
            int ret;
            EINTR_LOOP(ret, eventfd_write(fds[0], value));
            return;
        }
#endif
        char c = 0;
        qt_safe_write(fds[1], &c, 1);
    }
}

int QEventDispatcherEpollPrivate::QThreadPipe::check(const epoll_event &pfd)
{
    Q_ASSERT(pfd.data.fd == fds[0]);

    char c[16];
    const int readyread = pfd.events & EPOLLIN;

    if (readyread) {
        // consume the data on the thread pipe so that
        // poll doesn't immediately return next time
#if defined(Q_OS_VXWORKS)
        ::read(fds[0], c, sizeof(c));
        ::ioctl(fds[0], FIOFLUSH, 0);
#else
#  ifndef QT_NO_EVENTFD
        if (fds[1] == -1) {
            // eventfd
            eventfd_t value;
            eventfd_read(fds[0], &value);
        } else
#  endif
        {
            while (::read(fds[0], c, sizeof(c)) > 0) {
                qWarning("Read ......");
            }  //!TODO
        }
#endif

        if (!wakeUps.testAndSetRelease(1, 0)) {
            // hopefully, this is dead code
            qWarning("QThreadPipe: internal error, wakeUps.testAndSetRelease(1, 0) failed!");
        }
    }

    return readyread;
}

QEventDispatcherEpollPrivate::QEventDispatcherEpollPrivate()
{
    if (Q_UNLIKELY(threadPipe.init() == false))
        qFatal("QEventDispatcherEpollPrivate(): Can not continue without a thread pipe");
    epfd = epoll_create(FD_SETSIZE);
}

QEventDispatcherEpollPrivate::~QEventDispatcherEpollPrivate()
{
    // cleanup timers
    qDeleteAll(timerList);
}

void QEventDispatcherEpollPrivate::setSocketNotifierPending(QSocketNotifier *notifier)
{
    Q_ASSERT(notifier);

    if (pendingNotifiers.contains(notifier))
        return;

    pendingNotifiers << notifier;
}

int QEventDispatcherEpollPrivate::activateTimers()
{
    return timerList.activateTimers();
}

void QEventDispatcherEpollPrivate::markPendingSocketNotifiers()
{
    for (const epoll_event &pfd : qAsConst(pollfds)) {
        if (pfd.data.fd < 0 || pfd.events == 0)
            continue;

        auto it = socketNotifiers.find(pfd.data.fd);
        Q_ASSERT(it != socketNotifiers.end());

        const QSocketNotifierSetEpoll &sn_set = it.value();

        static const struct {
            QSocketNotifier::Type type;
            short flags;
        } notifiers[] = {
            { QSocketNotifier::Read,      POLLIN  | POLLHUP | POLLERR },
            { QSocketNotifier::Write,     POLLOUT | POLLHUP | POLLERR },
            { QSocketNotifier::Exception, POLLPRI | POLLHUP | POLLERR }
        };

        for (const auto &n : notifiers) {
            QSocketNotifier *notifier = sn_set.notifiers[n.type];

            if (!notifier)
                continue;

            // EPOLLNVAL-fd没有打开, POLLNVAL 从关闭的文件描述符读将触发，等价于 EBADF
            if (pfd.events & EPOLLRDHUP) {
                qWarning("QSocketNotifier: Invalid socket %d with type %s, disabling...",
                         it.key(), socketType(n.type));
                notifier->setEnabled(false);
            }

            if (pfd.events & n.flags)
                setSocketNotifierPending(notifier);
        }
    }

    pollfds.clear();
}

int QEventDispatcherEpollPrivate::activateSocketNotifiers()
{
    markPendingSocketNotifiers();

    if (pendingNotifiers.isEmpty())
        return 0;

    int n_activated = 0;
    QEvent event(QEvent::SockAct);

    while (!pendingNotifiers.isEmpty()) {
        QSocketNotifier *notifier = pendingNotifiers.takeFirst();
        QCoreApplication::sendEvent(notifier, &event);
        ++n_activated;
    }

    return n_activated;
}

QEventDispatcherEpoll::QEventDispatcherEpoll(QObject *parent)
    : QAbstractEventDispatcher(*new QEventDispatcherEpollPrivate, parent)
{ }

QEventDispatcherEpoll::QEventDispatcherEpoll(QEventDispatcherEpollPrivate &dd, QObject *parent)
    : QAbstractEventDispatcher(dd, parent)
{ }

QEventDispatcherEpoll::~QEventDispatcherEpoll()
{ }

/*!
    \internal
*/
void QEventDispatcherEpoll::registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *obj)
{
#ifndef QT_NO_DEBUG
    if (timerId < 1 || interval < 0 || !obj) {
        qWarning("QEventDispatcherEpoll::registerTimer: invalid arguments");
        return;
    } else if (obj->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QEventDispatcherEpoll::registerTimer: timers cannot be started from another thread");
        return;
    }
#endif

    Q_D(QEventDispatcherEpoll);
    d->timerList.registerTimer(timerId, interval, timerType, obj);
}

/*!
    \internal
*/
bool QEventDispatcherEpoll::unregisterTimer(int timerId)
{
#ifndef QT_NO_DEBUG
    if (timerId < 1) {
        qWarning("QEventDispatcherEpoll::unregisterTimer: invalid argument");
        return false;
    } else if (thread() != QThread::currentThread()) {
        qWarning("QEventDispatcherEpoll::unregisterTimer: timers cannot be stopped from another thread");
        return false;
    }
#endif

    Q_D(QEventDispatcherEpoll);
    return d->timerList.unregisterTimer(timerId);
}

/*!
    \internal
*/
bool QEventDispatcherEpoll::unregisterTimers(QObject *object)
{
#ifndef QT_NO_DEBUG
    if (!object) {
        qWarning("QEventDispatcherEpoll::unregisterTimers: invalid argument");
        return false;
    } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QEventDispatcherEpoll::unregisterTimers: timers cannot be stopped from another thread");
        return false;
    }
#endif

    Q_D(QEventDispatcherEpoll);
    return d->timerList.unregisterTimers(object);
}

QList<QEventDispatcherEpoll::TimerInfo>
QEventDispatcherEpoll::registeredTimers(QObject *object) const
{
    if (!object) {
        qWarning("QEventDispatcherEpoll:registeredTimers: invalid argument");
        return QList<TimerInfo>();
    }

    Q_D(const QEventDispatcherEpoll);
    return d->timerList.registeredTimers(object);
}

/*****************************************************************************
 QEventDispatcher implementations for UNIX
 *****************************************************************************/

void QEventDispatcherEpoll::registerSocketNotifier(QSocketNotifier *notifier)
{
    Q_ASSERT(notifier);
    int sockfd = notifier->socket();
    QSocketNotifier::Type type = notifier->type();
#ifndef QT_NO_DEBUG
    if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
        return;
    }
#endif

    Q_D(QEventDispatcherEpoll);

    qt_epollfd_ctl(type, d->epfd, sockfd, EPOLL_CTL_ADD);

    QSocketNotifierSetEpoll &sn_set = d->socketNotifiers[sockfd];

    if (sn_set.notifiers[type] && sn_set.notifiers[type] != notifier)
        qWarning("%s: Multiple socket notifiers for same socket %d and type %s",
                 Q_FUNC_INFO, sockfd, socketType(type));

    sn_set.notifiers[type] = notifier;
}

void QEventDispatcherEpoll::unregisterSocketNotifier(QSocketNotifier *notifier)
{
    Q_ASSERT(notifier);
    int sockfd = notifier->socket();
    QSocketNotifier::Type type = notifier->type();
#ifndef QT_NO_DEBUG
    if (notifier->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifier (fd %d) cannot be disabled from another thread.\n"
                "(Notifier's thread is %s(%p), event dispatcher's thread is %s(%p), current thread is %s(%p))",
                sockfd,
                notifier->thread() ? notifier->thread()->metaObject()->className() : "QThread", notifier->thread(),
                thread() ? thread()->metaObject()->className() : "QThread", thread(),
                QThread::currentThread() ? QThread::currentThread()->metaObject()->className() : "QThread", QThread::currentThread());
        return;
    }
#endif

    Q_D(QEventDispatcherEpoll);

    qt_epollfd_ctl(type, d->epfd, sockfd, EPOLL_CTL_DEL);

    d->pendingNotifiers.removeOne(notifier);

    auto i = d->socketNotifiers.find(sockfd);
    if (i == d->socketNotifiers.end())
        return;

    QSocketNotifierSetEpoll &sn_set = i.value();

    if (sn_set.notifiers[type] == nullptr)
        return;

    if (sn_set.notifiers[type] != notifier) {
        qWarning("%s: Multiple socket notifiers for same socket %d and type %s",
                 Q_FUNC_INFO, sockfd, socketType(type));
        return;
    }

    sn_set.notifiers[type] = nullptr;

    if (sn_set.isEmpty())
        d->socketNotifiers.erase(i);
}

bool QEventDispatcherEpoll::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherEpoll);
    d->interrupt.store(0);

    // we are awake, broadcast it
    Q_EMIT awake();
    QCoreApplicationPrivate::sendPostedEvents(0, 0, d->threadData);

    const bool include_timers = (flags & QEventLoop::X11ExcludeTimers) == 0;
    const bool include_notifiers = (flags & QEventLoop::ExcludeSocketNotifiers) == 0;
    const bool wait_for_events = flags & QEventLoop::WaitForMoreEvents;

    const bool canWait = (d->threadData->canWaitLocked()
                          && !d->interrupt.load()
                          && wait_for_events);

    if (canWait)
        Q_EMIT aboutToBlock();

    if (d->interrupt.load())
        return false;

    timespec *tm = nullptr;
    timespec wait_tm = { 0, 0 };

    if (!canWait || (include_timers && d->timerList.timerWait(wait_tm)))
        tm = &wait_tm;

    d->pollfds.clear();
    d->pollfds.reserve(1 + (include_notifiers ? d->socketNotifiers.size() : 0));

    if (include_notifiers)
        for (auto it = d->socketNotifiers.cbegin(); it != d->socketNotifiers.cend(); ++it) {
            d->pollfds.append(qt_make_epollfd(it.key(), it.value().events()));
        }

    // This must be last, as it's popped off the end below
    d->pollfds.append(d->threadPipe.prepare());

    int nevents = 0;
    switch (qt_safe_epoll(d->epfd, d->pollfds.data(), d->pollfds.size(), tm)) {
    case -1:
        perror("qt_safe_epoll");
        break;
    case 0:
        break;
    default:
        nevents += d->threadPipe.check(d->pollfds.takeLast());
        if (include_notifiers)
            nevents += d->activateSocketNotifiers();
        break;
    }

    if (include_timers)
        nevents += d->activateTimers();

    // return true if we handled events, false otherwise
    return (nevents > 0);
}

bool QEventDispatcherEpoll::hasPendingEvents()
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return qGlobalPostedEventsCount();
}

int QEventDispatcherEpoll::remainingTime(int timerId)
{
#ifndef QT_NO_DEBUG
    if (timerId < 1) {
        qWarning("QEventDispatcherEpoll::remainingTime: invalid argument");
        return -1;
    }
#endif

    Q_D(QEventDispatcherEpoll);
    return d->timerList.timerRemainingTime(timerId);
}

void QEventDispatcherEpoll::wakeUp()
{
    Q_D(QEventDispatcherEpoll);
    d->threadPipe.wakeUp();
}

void QEventDispatcherEpoll::interrupt()
{
    Q_D(QEventDispatcherEpoll);
    d->interrupt.store(1);
    wakeUp();
}

void QEventDispatcherEpoll::flush()
{ }

QT_END_NAMESPACE
