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
#include "util/dtimeunitformatter.h"
#include "filesystem/dtrashmanager.h"

DCORE_USE_NAMESPACE

class ut_DTimeUnitFormatter : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    DTimeUnitFormatter timeUnitFormatter;
};

void ut_DTimeUnitFormatter::SetUp()
{
}

void ut_DTimeUnitFormatter::TearDown()
{
}

TEST_F(ut_DTimeUnitFormatter, testDTimeUnitFormatterFormatAs)
{
    qreal result0 = timeUnitFormatter.formatAs(120, DTimeUnitFormatter::Seconds, DTimeUnitFormatter::Minute);
    ASSERT_TRUE(qFuzzyCompare(result0, 2));
    qreal result1 = timeUnitFormatter.formatAs(2, DTimeUnitFormatter::Minute, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(result1, 120));
    qreal result2 = timeUnitFormatter.formatAs(2, DTimeUnitFormatter::Seconds, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(result2, 2));
}

TEST_F(ut_DTimeUnitFormatter, testDTimeUnitFormatterFormat)
{
    QPair<double, int> result = timeUnitFormatter.format(120, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(result.first, 2));
    ASSERT_EQ(result.second, DTimeUnitFormatter::Minute);
}

TEST_F(ut_DTimeUnitFormatter, testDTimeUnitFormatterFormatAsUnitList)
{
    QList<QPair<double, int>> result = timeUnitFormatter.formatAsUnitList(121, DTimeUnitFormatter::Seconds);
    ASSERT_TRUE(qFuzzyCompare(result[0].first, 121));
    ASSERT_EQ(result[0].second, DTimeUnitFormatter::Seconds);
}

TEST_F(ut_DTimeUnitFormatter, testDTimeUnitFormatterUnitStr)
{
    ASSERT_EQ(timeUnitFormatter.unitStr(DTimeUnitFormatter::Seconds), "s");
    ASSERT_EQ(timeUnitFormatter.unitStr(DTimeUnitFormatter::Minute), "m");
    ASSERT_EQ(timeUnitFormatter.unitStr(DTimeUnitFormatter::Hour), "h");
    ASSERT_EQ(timeUnitFormatter.unitStr(DTimeUnitFormatter::Day), "d");
}
