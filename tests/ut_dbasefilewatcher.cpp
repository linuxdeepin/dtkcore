// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filesystem/dbasefilewatcher.h"
#include "private/dbasefilewatcher_p.h"

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>

DCORE_USE_NAMESPACE

class TestWatcherPrivate : public DBaseFileWatcherPrivate
{
public:
    TestWatcherPrivate(DBaseFileWatcher *qq)
        : DBaseFileWatcherPrivate(qq)
    {
    }

    bool start() override
    {
        m_startCalled++;
        return true;
    }

    bool stop() override
    {
        m_stopCalled++;
        return true;
    }

    int m_startCalled = 0;
    int m_stopCalled = 0;
    QUrl m_lastSubfileUrl;
    bool m_lastSubfileEnabled = false;
};

class TestWatcher : public DBaseFileWatcher
{
public:
    TestWatcher(const QUrl &url, QObject *parent = nullptr)
        : DBaseFileWatcher(*new TestWatcherPrivate(this), url, parent)
    {
    }

    void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true) override
    {
        TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(d_d_ptr.data());
        d->m_lastSubfileUrl = subfileUrl;
        d->m_lastSubfileEnabled = enabled;
        m_setEnabledCalled++;
    }

    int m_setEnabledCalled = 0;
};

class ut_DBaseFileWatcher : public testing::Test
{
protected:
    void SetUp() override
    {
        watcher = new TestWatcher(QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_test_watcher")));
    }

    void TearDown() override
    {
        delete watcher;
    }

    TestWatcher *watcher = nullptr;
};

TEST_F(ut_DBaseFileWatcher, fileUrl)
{
    EXPECT_EQ(watcher->fileUrl(), QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_test_watcher")));
}

TEST_F(ut_DBaseFileWatcher, startWatcher)
{
    EXPECT_TRUE(watcher->startWatcher());
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_EQ(d->m_startCalled, 1);
}

TEST_F(ut_DBaseFileWatcher, startWatcherAlreadyStarted)
{
    EXPECT_TRUE(watcher->startWatcher());
    EXPECT_TRUE(watcher->startWatcher());
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_EQ(d->m_startCalled, 1);
}

TEST_F(ut_DBaseFileWatcher, stopWatcherNotStarted)
{
    EXPECT_FALSE(watcher->stopWatcher());
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_EQ(d->m_stopCalled, 0);
}

TEST_F(ut_DBaseFileWatcher, stopWatcherAfterStart)
{
    watcher->startWatcher();
    EXPECT_TRUE(watcher->stopWatcher());
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_EQ(d->m_stopCalled, 1);
}

TEST_F(ut_DBaseFileWatcher, restartWatcher)
{
    watcher->startWatcher();
    EXPECT_TRUE(watcher->restartWatcher());
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_EQ(d->m_startCalled, 2);
    EXPECT_EQ(d->m_stopCalled, 1);
}

TEST_F(ut_DBaseFileWatcher, restartWatcherNotStarted)
{
    EXPECT_FALSE(watcher->restartWatcher());
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_EQ(d->m_startCalled, 0);
    EXPECT_EQ(d->m_stopCalled, 0);
}

TEST_F(ut_DBaseFileWatcher, setEnabledSubfileWatcher)
{
    QUrl subUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_test_watcher/subfile"));
    watcher->setEnabledSubfileWatcher(subUrl, true);
    EXPECT_EQ(watcher->m_setEnabledCalled, 1);
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_EQ(d->m_lastSubfileUrl, subUrl);
    EXPECT_TRUE(d->m_lastSubfileEnabled);
}

TEST_F(ut_DBaseFileWatcher, setEnabledSubfileWatcherDisabled)
{
    QUrl subUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_test_watcher/subfile"));
    watcher->setEnabledSubfileWatcher(subUrl, false);
    EXPECT_EQ(watcher->m_setEnabledCalled, 1);
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_FALSE(d->m_lastSubfileEnabled);
}

TEST_F(ut_DBaseFileWatcher, setEnabledSubfileWatcherMultipleCalls)
{
    QUrl url1 = QUrl::fromLocalFile(QStringLiteral("/tmp/test1"));
    QUrl url2 = QUrl::fromLocalFile(QStringLiteral("/tmp/test2"));
    watcher->setEnabledSubfileWatcher(url1, true);
    watcher->setEnabledSubfileWatcher(url2, false);
    EXPECT_EQ(watcher->m_setEnabledCalled, 2);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalSignalType1)
{
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileDeleted);
    QUrl targetUrl = watcher->fileUrl();
    QUrl argUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_test_watcher/deleted"));

    bool ok = DBaseFileWatcher::ghostSignal(targetUrl, &DBaseFileWatcher::fileDeleted, argUrl);
    EXPECT_TRUE(ok);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toUrl(), argUrl);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalSignalType1NoMatch)
{
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileDeleted);
    QUrl wrongUrl = QUrl::fromLocalFile(QStringLiteral("/nonexistent_watcher_url"));
    QUrl argUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/test"));

    bool ok = DBaseFileWatcher::ghostSignal(wrongUrl, &DBaseFileWatcher::fileDeleted, argUrl);
    EXPECT_FALSE(ok);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalSignalType1NullSignal)
{
    QUrl targetUrl = watcher->fileUrl();
    QUrl argUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/test"));

    bool ok = DBaseFileWatcher::ghostSignal(targetUrl, nullptr, argUrl);
    EXPECT_FALSE(ok);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalSignalType2)
{
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileMoved);
    QUrl targetUrl = watcher->fileUrl();
    QUrl fromUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_test_watcher/from"));
    QUrl toUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_test_watcher/to"));

    bool ok = DBaseFileWatcher::ghostSignal(targetUrl, &DBaseFileWatcher::fileMoved, fromUrl, toUrl);
    EXPECT_TRUE(ok);
    EXPECT_EQ(spy.count(), 1);
    auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toUrl(), fromUrl);
    EXPECT_EQ(args.at(1).toUrl(), toUrl);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalSignalType2NoMatch)
{
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileMoved);
    QUrl wrongUrl = QUrl::fromLocalFile(QStringLiteral("/nonexistent_watcher_url"));
    QUrl fromUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/from"));
    QUrl toUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/to"));

    bool ok = DBaseFileWatcher::ghostSignal(wrongUrl, &DBaseFileWatcher::fileMoved, fromUrl, toUrl);
    EXPECT_FALSE(ok);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalSignalType2NullSignal)
{
    QUrl targetUrl = watcher->fileUrl();
    QUrl fromUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/from"));
    QUrl toUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/to"));

    bool ok = DBaseFileWatcher::ghostSignal(targetUrl, nullptr, fromUrl, toUrl);
    EXPECT_FALSE(ok);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalFileAttributeChanged)
{
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileAttributeChanged);
    QUrl targetUrl = watcher->fileUrl();
    QUrl argUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/test_attr"));

    bool ok = DBaseFileWatcher::ghostSignal(targetUrl, &DBaseFileWatcher::fileAttributeChanged, argUrl);
    EXPECT_TRUE(ok);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalSubfileCreated)
{
    QSignalSpy spy(watcher, &DBaseFileWatcher::subfileCreated);
    QUrl targetUrl = watcher->fileUrl();
    QUrl argUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/test_sub"));

    bool ok = DBaseFileWatcher::ghostSignal(targetUrl, &DBaseFileWatcher::subfileCreated, argUrl);
    EXPECT_TRUE(ok);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalFileModified)
{
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileModified);
    QUrl targetUrl = watcher->fileUrl();
    QUrl argUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/test_mod"));

    bool ok = DBaseFileWatcher::ghostSignal(targetUrl, &DBaseFileWatcher::fileModified, argUrl);
    EXPECT_TRUE(ok);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DBaseFileWatcher, ghostSignalFileClosed)
{
    QSignalSpy spy(watcher, &DBaseFileWatcher::fileClosed);
    QUrl targetUrl = watcher->fileUrl();
    QUrl argUrl = QUrl::fromLocalFile(QStringLiteral("/tmp/test_closed"));

    bool ok = DBaseFileWatcher::ghostSignal(targetUrl, &DBaseFileWatcher::fileClosed, argUrl);
    EXPECT_TRUE(ok);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DBaseFileWatcher, destructorRemovesFromStaticList)
{
    int countBefore = DBaseFileWatcherPrivate::watcherList.size();
    {
        TestWatcher tempWatcher(QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_temp_watcher")));
        EXPECT_EQ(DBaseFileWatcherPrivate::watcherList.size(), countBefore + 1);
    }
    EXPECT_EQ(DBaseFileWatcherPrivate::watcherList.size(), countBefore);
}

TEST_F(ut_DBaseFileWatcher, destructorCallsStopWatcher)
{
    TestWatcher *w = new TestWatcher(QUrl::fromLocalFile(QStringLiteral("/tmp/dcap_destructor_test")));
    w->startWatcher();
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(w->d_d_ptr.data());
    int stopCountBefore = d->m_stopCalled;
    delete w;
    SUCCEED();
}

TEST_F(ut_DBaseFileWatcher, startStopStartCycle)
{
    EXPECT_TRUE(watcher->startWatcher());
    EXPECT_TRUE(watcher->stopWatcher());
    EXPECT_TRUE(watcher->startWatcher());
    TestWatcherPrivate *d = reinterpret_cast<TestWatcherPrivate *>(watcher->d_d_ptr.data());
    EXPECT_EQ(d->m_startCalled, 2);
    EXPECT_EQ(d->m_stopCalled, 1);
}
