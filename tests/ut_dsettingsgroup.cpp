// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settings/dsettingsgroup.h"
#include "settings/dsettingsoption.h"

#include <gtest/gtest.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QPointer>

DCORE_USE_NAMESPACE

class ut_DSettingsGroup : public testing::Test
{
protected:
    void SetUp() override
    {
        basicGroup["key"] = QStringLiteral("root");
        basicGroup["name"] = QStringLiteral("Root Group");
    }
    void TearDown() override
    {
        // fromJson() hands back a non-owning QPointer and the created group has no
        // parent, so the test owns it: release it here or ASan reports a leak.
        for (auto *group : m_owned)
            delete group;
        m_owned.clear();
    }

    template <class T>
    QPointer<T> own(QPointer<T> p)
    {
        m_owned.append(p.data());
        return p;
    }

    QList<QObject *> m_owned;
    QJsonObject basicGroup;
};

TEST_F(ut_DSettingsGroup, fromJsonBasic)
{
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->key(), QStringLiteral("root"));
    EXPECT_EQ(group->name(), QStringLiteral("Root Group"));
}

TEST_F(ut_DSettingsGroup, fromJsonWithPrefix)
{
    auto group = own(DSettingsGroup::fromJson(QStringLiteral("prefix"), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->key(), QStringLiteral("prefix.root"));
}

TEST_F(ut_DSettingsGroup, defaultConstructor)
{
    DSettingsGroup group;
    EXPECT_TRUE(group.key().isEmpty());
    EXPECT_TRUE(group.name().isEmpty());
    EXPECT_FALSE(group.isHidden());
}

TEST_F(ut_DSettingsGroup, isHiddenDefaultFalse)
{
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_FALSE(group->isHidden());
}

TEST_F(ut_DSettingsGroup, isHiddenExplicitTrue)
{
    basicGroup["hide"] = true;
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_TRUE(group->isHidden());
}

TEST_F(ut_DSettingsGroup, isHiddenExplicitFalse)
{
    basicGroup["hide"] = false;
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_FALSE(group->isHidden());
}

TEST_F(ut_DSettingsGroup, parentGroupDefaultNull)
{
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->parentGroup(), nullptr);
}

TEST_F(ut_DSettingsGroup, setParentGroup)
{
    auto parent = own(DSettingsGroup::fromJson(QString(), basicGroup));
    auto child = own(DSettingsGroup::fromJson(QStringLiteral("parent"), basicGroup));
    child->setParentGroup(parent);
    EXPECT_EQ(child->parentGroup(), parent);
}

TEST_F(ut_DSettingsGroup, childOptionsEmpty)
{
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_TRUE(group->childOptions().isEmpty());
}

TEST_F(ut_DSettingsGroup, childGroupsEmpty)
{
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_TRUE(group->childGroups().isEmpty());
}

TEST_F(ut_DSettingsGroup, optionsEmpty)
{
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_TRUE(group->options().isEmpty());
}

TEST_F(ut_DSettingsGroup, fromJsonWithOptions)
{
    QJsonArray options;
    QJsonObject opt1;
    opt1["key"] = QStringLiteral("opt1");
    opt1["name"] = QStringLiteral("Option 1");
    opt1["type"] = QStringLiteral("checkbox");
    opt1["default"] = true;
    options.append(opt1);
    basicGroup["options"] = options;

    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->childOptions().size(), 1);
    EXPECT_EQ(group->options().size(), 1);
    EXPECT_EQ(group->childOptions().first()->key(), QStringLiteral("root.opt1"));
    EXPECT_EQ(group->option(QStringLiteral("root.opt1"))->name(), QStringLiteral("Option 1"));
}

TEST_F(ut_DSettingsGroup, fromJsonWithMultipleOptions)
{
    QJsonArray options;
    QJsonObject opt1;
    opt1["key"] = QStringLiteral("opt1");
    opt1["type"] = QStringLiteral("checkbox");
    QJsonObject opt2;
    opt2["key"] = QStringLiteral("opt2");
    opt2["type"] = QStringLiteral("lineedit");
    options.append(opt1);
    options.append(opt2);
    basicGroup["options"] = options;

    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->childOptions().size(), 2);
    EXPECT_EQ(group->childOptions().at(0)->key(), QStringLiteral("root.opt1"));
    EXPECT_EQ(group->childOptions().at(1)->key(), QStringLiteral("root.opt2"));
}

TEST_F(ut_DSettingsGroup, fromJsonWithChildGroup)
{
    QJsonObject childGroup;
    childGroup["key"] = QStringLiteral("child");
    childGroup["name"] = QStringLiteral("Child Group");
    QJsonArray groups;
    groups.append(childGroup);
    basicGroup["groups"] = groups;

    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->childGroups().size(), 1);
    auto child = group->childGroup(QStringLiteral("root.child"));
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->key(), QStringLiteral("root.child"));
    EXPECT_EQ(child->name(), QStringLiteral("Child Group"));
}

TEST_F(ut_DSettingsGroup, fromJsonWithChildGroupAndParentLink)
{
    QJsonObject childGroup;
    childGroup["key"] = QStringLiteral("child");
    childGroup["name"] = QStringLiteral("Child");
    QJsonArray groups;
    groups.append(childGroup);
    basicGroup["groups"] = groups;

    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    auto child = group->childGroup(QStringLiteral("root.child"));
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->parentGroup(), group);
}

TEST_F(ut_DSettingsGroup, fromJsonChildGroupWithOptionsRecursiveMerge)
{
    QJsonObject childOpt;
    childOpt["key"] = QStringLiteral("copt");
    childOpt["type"] = QStringLiteral("checkbox");
    QJsonArray childOptions;
    childOptions.append(childOpt);
    QJsonObject childGroup;
    childGroup["key"] = QStringLiteral("child");
    childGroup["name"] = QStringLiteral("Child");
    childGroup["options"] = childOptions;
    QJsonArray groups;
    groups.append(childGroup);
    basicGroup["groups"] = groups;

    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    auto child = group->childGroup(QStringLiteral("root.child"));
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->childOptions().size(), 1);
    EXPECT_EQ(child->options().size(), 1);
    EXPECT_EQ(group->options().size(), 1);
    EXPECT_EQ(group->options().first()->key(), QStringLiteral("root.child.copt"));
}

TEST_F(ut_DSettingsGroup, fromJsonThreeLevelNesting)
{
    QJsonObject grandchildOpt;
    grandchildOpt["key"] = QStringLiteral("gopt");
    grandchildOpt["type"] = QStringLiteral("checkbox");
    QJsonArray grandchildOptions;
    grandchildOptions.append(grandchildOpt);

    QJsonObject grandchildGroup;
    grandchildGroup["key"] = QStringLiteral("grandchild");
    grandchildGroup["name"] = QStringLiteral("Grandchild");
    grandchildGroup["options"] = grandchildOptions;

    QJsonArray childGroups;
    childGroups.append(grandchildGroup);

    QJsonObject childGroup;
    childGroup["key"] = QStringLiteral("child");
    childGroup["name"] = QStringLiteral("Child");
    childGroup["groups"] = childGroups;

    QJsonArray rootGroups;
    rootGroups.append(childGroup);
    basicGroup["groups"] = rootGroups;

    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    auto child = group->childGroup(QStringLiteral("root.child"));
    ASSERT_NE(child, nullptr);
    auto grandchild = child->childGroup(QStringLiteral("root.child.grandchild"));
    ASSERT_NE(grandchild, nullptr);
    EXPECT_EQ(grandchild->childOptions().size(), 1);
    EXPECT_EQ(group->options().size(), 1);
    EXPECT_EQ(group->options().first()->key(), QStringLiteral("root.child.grandchild.gopt"));
}

TEST_F(ut_DSettingsGroup, childGroupNonExistent)
{
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->childGroup(QStringLiteral("nonexistent")), nullptr);
}

TEST_F(ut_DSettingsGroup, optionNonExistent)
{
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->option(QStringLiteral("nonexistent")), nullptr);
}

TEST_F(ut_DSettingsGroup, DISABLED_fromJsonEmptyKey)
{
    basicGroup.remove("key");
    basicGroup["key"] = QString();
    auto group = own(DSettingsGroup::fromJson(QString(), basicGroup));
    ASSERT_NE(group, nullptr);
    EXPECT_TRUE(group->key().isEmpty());
}
