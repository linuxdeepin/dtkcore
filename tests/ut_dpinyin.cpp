// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "util/dpinyin.h"

DCORE_USE_NAMESPACE


class ut_DPinyin : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void ut_DPinyin::SetUp()
{
}

void ut_DPinyin::TearDown()
{
}

TEST_F(ut_DPinyin, testChinese2Pinyin)
{
    auto pinyin = Chinese2Pinyin("你好, world");
    ASSERT_EQ(pinyin, "ni3hao3, world");
}
