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

#include "util/dasync.h"
#include "util/dthreadutils.h"

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

DCORE_USE_NAMESPACE

#define XLog() qDebug() << __LINE__ << " "

#define RUN_IN_SUB_THREAD 1

#define THREAD_BEGIN    std::thread thread([&]{
#define THREAD_END      }); thread.detach();

#if RUN_IN_SUB_THREAD
# define OPT_THREAD_BEGIN   THREAD_BEGIN
# define OPT_THREAD_END     THREAD_END
#else
# define OPT_THREAD_BEGIN
# define OPT_THREAD_END
#endif

#define TIMED_EXIT(second, loop) QTimer::singleShot(second * 1000, [&]{ loop.exit(); })

struct Configure
{
    QObject *o = nullptr;
    QWidget *w = nullptr;
} conf;
static Configure *config = &conf;

int main1(int argc, char *argv[]);
int main2(int argc, char *argv[]);
int main3(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 将以下所有新建的对象都托管给 w、o
    config->w = new QWidget;
    config->o = new QObject;

    config->w->show();

    main1(argc, argv);
    main2(argc, argv);
    main3(argc, argv);

    QTimer::singleShot(1 * 1000, [&]{
        config->w->deleteLater();
        config->o->deleteLater();
    });

    QTimer::singleShot(2 * 1000, [&]{
        qApp->exit(0);
    });

    qDebug() << "finished xxxxxxxxxxxxxxxxxxxxxxxxxxxx";

    return app.exec();
}

#pragma mark main1 ------------------------------------------------------------

// 这是一个最基础的示例程序
DAsync<int, int> *testTask() {
    auto task = new DAsync<int, int>(config->o);

    int i = 0;
    while (i < 100) {
        task->postData(i++);
    }

    task->post([](int arg) {

        Q_ASSERT(!D_THREAD_IN_MAIN());
        XLog() << "run in child thread: " << arg;
        return arg * 2;

    })->then([&](int arg) {

        Q_ASSERT(D_THREAD_IN_MAIN());
        XLog() << "run in main thread:" << arg;

    })->start();

    task->postData(i++);

    return task;
}

void runTest()
{
    OPT_THREAD_BEGIN
        auto task = testTask();
        // 删除前应该先等待所有的任务执行完或取消未执行的任务
        // 主线程中只能用 isFinished 查询状态，用 cancelAll 取消之后的任务队列
        // 其它子线程中（非 post、非 then 函数）中可以直接 waitForFinished 然后删除
        // 也可以使用 task->setParent 去托管，自动释放
        if (!D_THREAD_IN_MAIN()) {
            task->waitForFinished(false);
            // task->deleteLater();
        }
    OPT_THREAD_END
}

int main1(int argc, char *argv[]) {
    QEventLoop loop;
    TIMED_EXIT(3, loop);

    XLog() << "in main thread: " << pthread_self();

    // DAsync 依赖事件循环，不能被阻塞，比如 thread.join 就不行
    // 运行在主线程中和运行在子线程中应该有一样的结果才对
    OPT_THREAD_BEGIN
        runTest();
    OPT_THREAD_END

    return loop.exec();
}

#pragma mark main2 ------------------------------------------------------------

int main2(int argc, char *argv[]) {
    QEventLoop loop;
    TIMED_EXIT(3, loop);

    OPT_THREAD_BEGIN
        QWidget *w = DThreadUtil::runInMainThread([&](){
            QWidget *w = new QWidget(config->w);
            w->setBackgroundRole(QPalette::HighlightedText);
            w->show();
            return w;
        });
        // w->show();
    OPT_THREAD_END

    return loop.exec();
}

#pragma mark main3 ------------------------------------------------------------

int test1();
int test2();
int test3();
int test4();
int test5();
int test6();
int test7();
int test8();

int main3(int argc, char *argv[]) {
    std::clog << "in main thread: " << pthread_self() << std::endl;

    // 示例 1，输入输出都是基本类型
    test1();

    // 示例 2，输入基本类型，输出复合类型
    test2();

    // 示例 3，输入输出都是复合类型
    test3();

    // 示例 4，输入输出都是自定义类型的指针
    test4();

    // 示例 5, 异步执行一个输入复合类型、没有输出的一次性任务，执行结束后通知主线程
    // 间歇性输入数据，要保证生产者消费者模型的正确性。
    test5();

    // 示例 6, 异步执行一个没有输入、输出参数的一次性任务，执行结束后通知在主线
    test6();

    // 示例 7, 异步运行一个没有输入参数的一次性任务，执行后在主线程处理结果
    test7();

    // 示例 8, 在子线程中异步创建一个 widget 并显示出来：
    test8();
    // std::thread thread([&]{ test8(); });
    // thread.detach();

    return 0;
}

int test1() {
    QEventLoop loop;
    TIMED_EXIT(2, loop);

    // 加 static 防止函数执行结束后线程中继续 postData 访问已经释放的栈上变量
    static auto task1 = new DAsync<int, int>(config->o);
    static int i = 0;

    while (i < 100) {
        task1->postData(i++);
    }

    task1->post([](int arg) {

        XLog() << "async task: " << arg;
        return arg * 2;

    })->then([](int arg) {

        XLog() << "get result: " << arg;

    })->start();

    return loop.exec();
}

int test2() {
    QEventLoop loop;

    static auto task2 = new DAsync<int, QString>(config->o);

    static int i = 0;
    static bool stopFlag = false;

    // TIMED_EXIT(3, loop);
    QTimer::singleShot(3 * 1000, [&]{
        stopFlag = true;
        loop.exit();
    });

    while(i < 100) {
        task2->postData(i++);
    }

    task2->post([](int arg) -> QString {

        XLog() << "async task: " << arg;
        return QString::number(arg);

    })->then([](QString arg) {

        XLog() << "get result: " << arg;

    })->start();

    THREAD_BEGIN
        while (!stopFlag && i < 220) {
            XLog() << "post data:  " << i;
            task2->postData(i++);
            usleep(200 * 1000);
        }
    THREAD_END

    // task2->waitForFinished();
    // task2->deleteLater();

    return loop.exec();
}

int test3() {
    QEventLoop loop;
    TIMED_EXIT(3, loop);

    static auto task3 = new DAsync<QString, QString>(config->o);

    static int i = 0;
    while (i < 100) {
        task3->postData(QString::number(i++));
    }

    task3->post([](QString arg) -> QString {

        XLog() << "async task: " << arg;
        return arg;

    })->then([](QString arg) {

        XLog() << "get result " << arg;

    })->start();

    // task3->waitForFinished();
    // task3->deleteLater();

    return loop.exec();
}

int test4() {
    QEventLoop loop;
    TIMED_EXIT(3, loop);

    class Test : public QObject {
    public:
        Test(int in, QObject *parent = nullptr)
            : QObject   (parent)
            , count     (in)
        {
        }
        int count = 0;
    };

    static auto task4 = new DAsync<Test *, Test*>(config->o);
    static int i = 0;

    while (i < 100) {
        task4->postData(new Test(i++, config->o));
    }

    task4->post([](Test *arg) -> Test * {

        XLog() << "async task: " << arg->count;
        return arg;

    })->then([](Test *arg) {

        XLog() << "get result " << arg->count;

    })->start();

    return loop.exec();
}

int test5() {
    QEventLoop loop;
    // TIMED_EXIT(3, loop);
    static bool stopFlag = false;
    QTimer::singleShot(3 * 1000, [&]{
        stopFlag = true;
        loop.exit();
    });

    static auto task5 = new DAsync<QString, void>(config->o);
    static int i = 0;

    while (i < 100) {
        task5->postData(QString::number(i++));
    }

    task5->post([](QString arg) {

        XLog() << "async task." << arg;

    })->then([]() {

        XLog() << "get void";

    })->start();

    OPT_THREAD_BEGIN
        while (!stopFlag) {
            usleep(200 * 1000);
            task5->postData(QString::number(i++));
        }
    OPT_THREAD_END

    return loop.exec();
}

int test6() {
    QEventLoop loop;
    TIMED_EXIT(1, loop);

    static auto task6 = new DAsync<void, void>(config->o);

    task6->post([]() {

        XLog() << "async task.";

    })->then([]() {

        XLog() << "get result.";

    })->start();

    // 如果只想在子线程执行一个任务，不需要主线程的任何处理，按照以下方式，
    // 其实也只是只设置一个函数就可以了：
    // task6->post([]() { XLog() << "async task."; });
    // task6->startUp();

    return loop.exec();
}

int test7() {
    QEventLoop loop;
    TIMED_EXIT(1, loop);

    static auto task7 = new DAsync<void, QString>(config->o);
    static int i = 0;

    task7->post([&]() {

        XLog() << "async task.";
        return QString("%1").arg(i++);

    })->then([](QString arg) {

        XLog() << "get result " << arg;

    })->start();

    return loop.exec();
}

int test8() {
    QEventLoop loop;
    TIMED_EXIT(1, loop);

    static auto task8 = new DAsync<void, QString>(config->o);

    // 注意，任务是异步执行的，传进去的一定不能是栈区变量！
    static int i = 0;
    task8->post([&]() -> QString {
        Q_ASSERT(!D_THREAD_IN_MAIN());
        QWidget *w = DThreadUtil::runInMainThread([](){
            Q_ASSERT(D_THREAD_IN_MAIN());
            QWidget *w = new QWidget(config->w);
            w->setBackgroundRole(QPalette::Text);
            w->show();
            return w;
        });

        // 在外面调用并不合适，虽然也能显示出来。比如 mac 上这么用就显示不出来
        // w->setBackgroundRole(QPalette::Text);
        // w->show();

        XLog() << "async task." << QString("%1").arg(i++);
        return QString("%1").arg(i++);

    })->then([](QString str) {

         XLog() << "get result " << str;

    })->start();

    return loop.exec();
}
