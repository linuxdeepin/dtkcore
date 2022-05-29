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

#include <QtCore/private/qglobal_p.h>
#include "epoll_global.h"
#include "qcore_unix_p.h"
#include "qelapsedtimer.h"
#include <QDebug>

#include <stdlib.h>

#ifdef __GLIBC__
#  include <sys/syscall.h>
#  include <pthread.h>
#  include <unistd.h>
#endif

QT_BEGIN_NAMESPACE

QByteArray qt_readlink(const char *path)
{
#ifndef PATH_MAX
    // suitably large value that won't consume too much memory
#  define PATH_MAX  1024*1024
#endif

    QByteArray buf(256, Qt::Uninitialized);

    ssize_t len = ::readlink(path, buf.data(), buf.size());
    while (len == buf.size()) {
        // readlink(2) will fill our buffer and not necessarily terminate with NUL;
        if (buf.size() >= PATH_MAX) {
            errno = ENAMETOOLONG;
            return QByteArray();
        }

        // double the size and try again
        buf.resize(buf.size() * 2);
        len = ::readlink(path, buf.data(), buf.size());
    }

    if (len == -1)
        return QByteArray();

    buf.resize(len);
    return buf;
}

static inline bool time_update(struct timespec *tv, const struct timespec &start,
                               const struct timespec &timeout)
{
    // clock source is (hopefully) monotonic, so we can recalculate how much timeout is left;
    // if it isn't monotonic, we'll simply hope that it hasn't jumped, because we have no alternative
    struct timespec now = qt_gettime();
    *tv = timeout + start - now;
    return tv->tv_sec >= 0;
}

static inline int qt_epoll(int epoll_fd, struct epoll_event *evs, nfds_t nfds, const struct timespec *timeout_ts)
{
    // default wait forever
    int ms = -1;
    if (timeout_ts) {
        ms = timeout_ts->tv_sec * 1000 + timeout_ts->tv_nsec / 1000 / 1000;
    }
    return ::epoll_wait(epoll_fd, evs, nfds, ms);
}

int qt_safe_epoll(int epoll_fd, struct epoll_event *evs, nfds_t nfds, const struct timespec *timeout_ts)
{
    if (!timeout_ts) {
        // no timeout -> block forever
        int ret;
        EINTR_LOOP(ret, qt_epoll(epoll_fd, evs, nfds, timeout_ts));
        return ret;
    }

    timespec start = qt_gettime();
    timespec timeout = *timeout_ts;

    // loop and recalculate the timeout as needed
    Q_FOREVER {
        const int ret = qt_epoll(epoll_fd, evs, nfds, timeout_ts);
        if (ret != -1 || errno != EINTR)
            return ret;

        // recalculate the timeout
        if (!time_update(&timeout, start, *timeout_ts)) {
            // timeout during update
            // or clock reset, fake timeout error
            return 0;
        }
    }
}

QT_END_NAMESPACE
