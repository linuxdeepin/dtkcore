/*
 * Copyright (C) 2021 ~ 2021 UnionTech Technology Co., Ltd.
 *
 * Author:     Wang Peng <993381@qq.com>
 *
 * Maintainer: Wang Peng <wangpenga@uniontech.com>
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
#include <QWidget>
#include <QTimer>
#include <iostream>
#include <QEventLoop>
#include <QApplication>

#include <unistd.h>

#include "util/dthreadutils.h"
// #include "util/depolldispatcher.h"
#include "util/qeventdispatcher_epoll.h"

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

DCORE_USE_NAMESPACE

#define XLog() qDebug() << __LINE__ << " "

#define D_THREAD_IN_MAIN() (qApp->instance() && qApp->instance()->thread() == QThread::currentThread())

class Worker : public QThread {
public:
    void run() override {
        QTimer::singleShot(1000, [this]{
            if (!D_THREAD_IN_MAIN()) {
                qInfo() << "Run in sub thread, dispatcher: " << eventDispatcher();
            }
        });
        exec();
    }
};

int main(int argc, char *argv[]) {
    QEventDispatcherEpoll *mainDispatcher = new QEventDispatcherEpoll;
    qInfo() << "mainDispatcher: " << mainDispatcher;
    QCoreApplication::setEventDispatcher(mainDispatcher);

    QCoreApplication app(argc, argv);

    QTimer::singleShot(1000, []{
        if (D_THREAD_IN_MAIN()) {
            qInfo() << "Run in main thread, dispatcher: " <<  QThread::currentThread()->eventDispatcher();
        }
    });

    Worker w;
    QEventDispatcherEpoll *subDispatcher = new QEventDispatcherEpoll;
    qInfo() << "subDispatcher: " << subDispatcher;
    w.setEventDispatcher(subDispatcher);
    w.start();

    return app.exec();
}
