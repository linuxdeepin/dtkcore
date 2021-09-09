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
#ifndef DTIMEDLOOP_H
#define DTIMEDLOOP_H
#include <dtkcore_global.h>
#include <DObject>

#include <QEventLoop>

DCORE_BEGIN_NAMESPACE

class DObject;
class DTimedLoopPrivate;
class DTimedLoop : public QEventLoop,  public DObject {
    Q_OBJECT
public:
    explicit DTimedLoop() noexcept;
    explicit DTimedLoop(QObject *parent) noexcept;

    ~DTimedLoop();

    // 如果是 isRunning 则返回从开始到现在的 exec 执行时间，否则返回上次运行的时间
    int runningTime();
    void setTimeDump(bool flag = true);

    void exit(int returnCode = 0);

    // 方式1：不传定时时间，如果不退出就一直执行，配合 exit 使用
    // 方式2：传入durationMs 参数的是定时执行的，也能调用 exit 提前退出
    // 如果传入了 executionName 就会为本次执行设置一个名字，会输出到 log
    // 在执行结束将会打印 exec 的执行时间，可以用 setTimeDump 控制其是否打印
    int exec(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    int exec(int durationMs, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    int exec(const QString &executionName, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);
    int exec(int durationMs, const QString &executionName, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);

private:
    Q_DISABLE_COPY(DTimedLoop)
    D_DECLARE_PRIVATE(DTimedLoop)
};

DCORE_END_NAMESPACE

#endif // DTIMEDLOOP_H
