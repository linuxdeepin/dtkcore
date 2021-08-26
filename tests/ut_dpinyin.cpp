/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Wang Fei <wangfeia@uniontech.com>
 *
 * Maintainer: Wang Fei <wangfeia@uniontech.com>
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
