// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#define private public
#include "dnotifysender.h"
#undef private

#include <QVariantMap>
#include <QStringList>
#include <QDBusPendingCall>

DCORE_USE_NAMESPACE

struct DUtil::DNotifyData {
    uint        m_replaceId;
    int         m_timeOut;
    QString     m_body;
    QString     m_summary;
    QString     m_appIcon;
    QString     m_appName;
    QStringList m_actions;
    QVariantMap m_hints;
};

TEST(ut_DNotifySender, constructorSummary)
{
    DUtil::DNotifySender sender("test summary");
    EXPECT_EQ(sender.m_dbusData->m_summary, "test summary");
}

TEST(ut_DNotifySender, constructorEmptySummary)
{
    DUtil::DNotifySender sender("");
    EXPECT_TRUE(sender.m_dbusData->m_summary.isEmpty());
}

TEST(ut_DNotifySender, defaultValues)
{
    DUtil::DNotifySender sender("summary");
    EXPECT_EQ(sender.m_dbusData->m_replaceId, 0u);
    EXPECT_EQ(sender.m_dbusData->m_timeOut, 0);
    EXPECT_TRUE(sender.m_dbusData->m_body.isEmpty());
    EXPECT_TRUE(sender.m_dbusData->m_appIcon.isEmpty());
    EXPECT_TRUE(sender.m_dbusData->m_appName.isEmpty());
    EXPECT_TRUE(sender.m_dbusData->m_actions.isEmpty());
    EXPECT_TRUE(sender.m_dbusData->m_hints.isEmpty());
}

TEST(ut_DNotifySender, appName)
{
    DUtil::DNotifySender sender("summary");
    sender.appName("MyApp");
    EXPECT_EQ(sender.m_dbusData->m_appName, "MyApp");
}

TEST(ut_DNotifySender, appNameDefault)
{
    DUtil::DNotifySender sender("summary");
    sender.appName("MyApp");
    sender.appName();
    EXPECT_TRUE(sender.m_dbusData->m_appName.isEmpty());
}

TEST(ut_DNotifySender, appIcon)
{
    DUtil::DNotifySender sender("summary");
    sender.appIcon("icon.png");
    EXPECT_EQ(sender.m_dbusData->m_appIcon, "icon.png");
}

TEST(ut_DNotifySender, appIconDefault)
{
    DUtil::DNotifySender sender("summary");
    sender.appIcon("icon.png");
    sender.appIcon();
    EXPECT_TRUE(sender.m_dbusData->m_appIcon.isEmpty());
}

TEST(ut_DNotifySender, appBody)
{
    DUtil::DNotifySender sender("summary");
    sender.appBody("body text");
    EXPECT_EQ(sender.m_dbusData->m_body, "body text");
}

TEST(ut_DNotifySender, appBodyDefault)
{
    DUtil::DNotifySender sender("summary");
    sender.appBody("body text");
    sender.appBody();
    EXPECT_TRUE(sender.m_dbusData->m_body.isEmpty());
}

TEST(ut_DNotifySender, replaceId)
{
    DUtil::DNotifySender sender("summary");
    sender.replaceId(123456);
    EXPECT_EQ(sender.m_dbusData->m_replaceId, 123456u);
}

TEST(ut_DNotifySender, replaceIdDefault)
{
    DUtil::DNotifySender sender("summary");
    sender.replaceId(999);
    sender.replaceId();
    EXPECT_EQ(sender.m_dbusData->m_replaceId, 0u);
}

TEST(ut_DNotifySender, timeOut)
{
    DUtil::DNotifySender sender("summary");
    sender.timeOut(5000);
    EXPECT_EQ(sender.m_dbusData->m_timeOut, 5000);
}

TEST(ut_DNotifySender, timeOutDefault)
{
    GTEST_SKIP() << "DNotifySender requires DBus notification service";
    DUtil::DNotifySender sender("summary");
    sender.timeOut(5000);
    sender.timeOut();
    // timeOut() default param is -1, so calling with no args resets m_timeOut to -1
    EXPECT_EQ(sender.m_dbusData->m_timeOut, -1);
}

TEST(ut_DNotifySender, actions)
{
    DUtil::DNotifySender sender("summary");
    QStringList acts{"1", "Open", "2", "Close"};
    sender.actions(acts);
    EXPECT_EQ(sender.m_dbusData->m_actions, acts);
}

TEST(ut_DNotifySender, actionsDefault)
{
    DUtil::DNotifySender sender("summary");
    sender.actions({"1", "2"});
    sender.actions();
    EXPECT_TRUE(sender.m_dbusData->m_actions.isEmpty());
}

TEST(ut_DNotifySender, hints)
{
    DUtil::DNotifySender sender("summary");
    QVariantMap hints;
    hints.insert("urgency", 2);
    hints.insert("category", "test");
    sender.hints(hints);
    EXPECT_EQ(sender.m_dbusData->m_hints, hints);
}

TEST(ut_DNotifySender, hintsDefault)
{
    DUtil::DNotifySender sender("summary");
    QVariantMap hints;
    hints.insert("urgency", 2);
    sender.hints(hints);
    sender.hints();
    EXPECT_TRUE(sender.m_dbusData->m_hints.isEmpty());
}

TEST(ut_DNotifySender, chainedMethods)
{
    DUtil::DNotifySender sender("summary");
    sender.appName("appName")
            .replaceId(123456)
            .appIcon("iconName")
            .appBody("msg body")
            .actions({"1", "2"})
            .timeOut(5000)
            .hints({{"urgency", 1}});

    EXPECT_EQ(sender.m_dbusData->m_summary, "summary");
    EXPECT_EQ(sender.m_dbusData->m_body, "msg body");
    EXPECT_EQ(sender.m_dbusData->m_replaceId, 123456u);
    EXPECT_EQ(sender.m_dbusData->m_appIcon, "iconName");
    EXPECT_EQ(sender.m_dbusData->m_appName, "appName");
    EXPECT_EQ(sender.m_dbusData->m_actions, QStringList({"1", "2"}));
    EXPECT_EQ(sender.m_dbusData->m_timeOut, 5000);
    EXPECT_EQ(sender.m_dbusData->m_hints.value("urgency").toInt(), 1);
}

TEST(ut_DNotifySender, call)
{
    DUtil::DNotifySender sender("summary");
    sender.appName("test").appBody("body").timeOut(100);
    QDBusPendingCall call = sender.call();
    EXPECT_FALSE(call.isError());
    call.waitForFinished();
}
