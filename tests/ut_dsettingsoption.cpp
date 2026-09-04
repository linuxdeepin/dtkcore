// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settings/dsettingsoption.h"
#include "settings/dsettingsgroup.h"

#include <gtest/gtest.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QList>
#include <QPointer>
#include <QSignalSpy>

DCORE_USE_NAMESPACE

class ut_DSettingsOption : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        jsonObj["key"] = QStringLiteral("test_key");
        jsonObj["name"] = QStringLiteral("Test Option");
        jsonObj["type"] = QStringLiteral("checkbox");
        jsonObj["default"] = true;
    }
    virtual void TearDown() override
    {
        // fromJson() hands back a non-owning QPointer and the created option has no
        // parent, so the test owns it: release it here or ASan reports a leak.
        for (auto *opt : m_owned)
            delete opt;
        m_owned.clear();
    }

    template <class T>
    QPointer<T> own(QPointer<T> p)
    {
        m_owned.append(p.data());
        return p;
    }

    QList<QObject *> m_owned;
    QJsonObject jsonObj;
};

TEST_F(ut_DSettingsOption, fromJsonBasic)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("prefix"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_EQ(opt->key(), QStringLiteral("prefix.test_key"));
    EXPECT_EQ(opt->name(), QStringLiteral("Test Option"));
    EXPECT_EQ(opt->viewType(), QStringLiteral("checkbox"));
}

TEST_F(ut_DSettingsOption, fromJsonDefaultValue)
{
    jsonObj["default"] = 42;
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_EQ(opt->defaultValue().toInt(), 42);
    EXPECT_EQ(opt->value().toInt(), 42);
}

TEST_F(ut_DSettingsOption, fromJsonStringDefault)
{
    jsonObj["default"] = QStringLiteral("hello");
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_EQ(opt->defaultValue().toString(), QStringLiteral("hello"));
    EXPECT_EQ(opt->value().toString(), QStringLiteral("hello"));
}

TEST_F(ut_DSettingsOption, fromJsonNoDefault)
{
    jsonObj.remove("default");
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_FALSE(opt->defaultValue().isValid());
}

TEST_F(ut_DSettingsOption, canResetDefaultTrue)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_TRUE(opt->canReset());
}

TEST_F(ut_DSettingsOption, canResetExplicitTrue)
{
    jsonObj["reset"] = true;
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_TRUE(opt->canReset());
}

TEST_F(ut_DSettingsOption, canResetExplicitFalse)
{
    jsonObj["reset"] = false;
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_FALSE(opt->canReset());
}

TEST_F(ut_DSettingsOption, isHiddenDefaultFalse)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_FALSE(opt->isHidden());
}

TEST_F(ut_DSettingsOption, isHiddenExplicitTrue)
{
    jsonObj["hide"] = true;
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_TRUE(opt->isHidden());
}

TEST_F(ut_DSettingsOption, isHiddenExplicitFalse)
{
    jsonObj["hide"] = false;
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_FALSE(opt->isHidden());
}

// 被测代码缺陷：DSettingsOptionPrivate 构造函数未初始化 canReset/hidden 成员
// （src/settings/dsettingsoption.cpp:28-29）。fromJson 路径下 parseJson 会显式赋值，
// 因此 fromJson 构造的用例不触发 UB；缺陷仅在绕过 fromJson 直接构造 DSettingsOption 时触发。
// 以下用例直接构造 DSettingsOption 会读取未初始化成员（UB），故跳过。
// TODO: 待被测代码修复后补充断言
TEST_F(ut_DSettingsOption, directConstructorUninitializedCanReset)
{
    GTEST_SKIP() << "Skipped: DSettingsOptionPrivate does not initialize canReset (UB, src/settings/dsettingsoption.cpp:28-29)";
}

TEST_F(ut_DSettingsOption, directConstructorUninitializedIsHidden)
{
    GTEST_SKIP() << "Skipped: DSettingsOptionPrivate does not initialize hidden (UB, src/settings/dsettingsoption.cpp:28-29)";
}

TEST_F(ut_DSettingsOption, setValueEmitsSignal)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);

    QSignalSpy spy(opt.data(), &DSettingsOption::valueChanged);
    opt->setValue(false);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toBool(), false);
}

TEST_F(ut_DSettingsOption, setValueNoSignalWhenSame)
{
    jsonObj["default"] = true;
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);

    QSignalSpy spy(opt.data(), &DSettingsOption::valueChanged);
    opt->setValue(true);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ut_DSettingsOption, valueReturnsSetValue)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);

    opt->setValue(false);
    EXPECT_EQ(opt->value().toBool(), false);

    opt->setValue(true);
    EXPECT_EQ(opt->value().toBool(), true);
}

TEST_F(ut_DSettingsOption, setDataEmitsSignal)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);

    QSignalSpy spy(opt.data(), &DSettingsOption::dataChanged);
    opt->setData(QStringLiteral("custom"), QVariant(123));
    EXPECT_EQ(spy.count(), 1);
    auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), QStringLiteral("custom"));
    EXPECT_EQ(args.at(1).toInt(), 123);
}

TEST_F(ut_DSettingsOption, setDataNoSignalWhenSame)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);

    opt->setData(QStringLiteral("key1"), QVariant("val1"));
    QSignalSpy spy(opt.data(), &DSettingsOption::dataChanged);
    opt->setData(QStringLiteral("key1"), QVariant("val1"));
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ut_DSettingsOption, dataReturnsSetValue)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);

    opt->setData(QStringLiteral("myData"), QVariant("testValue"));
    EXPECT_EQ(opt->data(QStringLiteral("myData")).toString(), QStringLiteral("testValue"));
}

TEST_F(ut_DSettingsOption, dataReturnsInvalidForUnsetKey)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);

    EXPECT_FALSE(opt->data(QStringLiteral("nonexistent")).isValid());
}

TEST_F(ut_DSettingsOption, viewType)
{
    jsonObj["type"] = QStringLiteral("combobox");
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_EQ(opt->viewType(), QStringLiteral("combobox"));
}

TEST_F(ut_DSettingsOption, viewTypeNotSpecified)
{
    jsonObj.remove("type");
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_TRUE(opt->viewType().isEmpty());
}

TEST_F(ut_DSettingsOption, keyWithPrefix)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("group1"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_EQ(opt->key(), QStringLiteral("group1.test_key"));
}

TEST_F(ut_DSettingsOption, customDataFromJson)
{
    jsonObj["extra1"] = QStringLiteral("value1");
    jsonObj["extra2"] = 42;
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_EQ(opt->data(QStringLiteral("extra1")).toString(), QStringLiteral("value1"));
    EXPECT_EQ(opt->data(QStringLiteral("extra2")).toInt(), 42);
}

TEST_F(ut_DSettingsOption, arrayDataFromJson)
{
    QJsonArray arr;
    arr.append("a");
    arr.append("b");
    arr.append("c");
    jsonObj["items"] = arr;
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    QStringList list = opt->data(QStringLiteral("items")).toStringList();
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list[0], QStringLiteral("a"));
    EXPECT_EQ(list[1], QStringLiteral("b"));
    EXPECT_EQ(list[2], QStringLiteral("c"));
}

TEST_F(ut_DSettingsOption, parentGroup)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);
    EXPECT_EQ(opt->parentGroup(), nullptr);

    // parentGroup is a QPointer, can set to a group-like object for coverage
    // but DSettingsGroup construction is complex; just verify it starts null
    opt->setParentGroup(nullptr);
    EXPECT_EQ(opt->parentGroup(), nullptr);
}

TEST_F(ut_DSettingsOption, multipleSetDataSameKey)
{
    auto opt = own(DSettingsOption::fromJson(QStringLiteral("grp"), jsonObj));
    ASSERT_NE(opt, nullptr);

    opt->setData(QStringLiteral("k"), QVariant("v1"));
    EXPECT_EQ(opt->data(QStringLiteral("k")).toString(), QStringLiteral("v1"));

    opt->setData(QStringLiteral("k"), QVariant("v2"));
    EXPECT_EQ(opt->data(QStringLiteral("k")).toString(), QStringLiteral("v2"));
}
