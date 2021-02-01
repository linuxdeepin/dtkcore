/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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

#include <QObject>
#include <QTest>
#include <QtConcurrent>

#include <util/DThreadUtils>

DCORE_USE_NAMESPACE

class tst_DThreadUtils : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCallInMainThread();
};

void tst_DThreadUtils::testCallInMainThread()
{
    QVERIFY(DThreadUtil::runInMainThread([] {
        return QThread::currentThread() == QCoreApplication::instance()->thread();
    }));

    auto result = QtConcurrent::run([] {
        QThread *t = QThread::currentThread();
        QVERIFY(DThreadUtil::runInMainThread([] (QThread *thread) {
            return QThread::currentThread() == QCoreApplication::instance()->thread() && QThread::currentThread() != thread;
        }, t));
    });

    QVERIFY(QTest::qWaitFor([&] {
        return result.isFinished();
    }));

    {
        // 测试target对象销毁后是否还会触发函数调用
        QPointer<QObject> object = new QObject();
        bool test = true;
        auto result1 = QtConcurrent::run([&test, object] {
            DThreadUtil::runInMainThread(object, [&test, object] () {
                if (!object)
                    return false;

                delete object.data();
                return true;
            });
        });
        auto result2 = QtConcurrent::run([&test, object] {
            DThreadUtil::runInMainThread(object, [&test, object] () {
                if (!object)
                    return false;

                delete object.data();
                return true;
            });
        });

        QVERIFY(QTest::qWaitFor([&] {
            return result1.isFinished() && result2.isFinished();
        }));
        QVERIFY(test);
    }
}

QTEST_MAIN(tst_DThreadUtils)

#include "ut_dthreadutils.moc"
