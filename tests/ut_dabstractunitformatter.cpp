// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "util/dabstractunitformatter.h"

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

class TestUnitFormatter : public DAbstractUnitFormatter
{
public:
    enum Units { Unit0, Unit1, Unit2, Unit3 };

protected:
    int unitMax() const override { return Unit3; }
    int unitMin() const override { return Unit0; }
    uint unitConvertRate(int unitId) const override
    {
        switch (unitId) {
        case Unit0: return 10;
        case Unit1: return 100;
        case Unit2: return 1000;
        default: return 1;
        }
    }
    QString unitStr(int unitId) const override
    {
        switch (unitId) {
        case Unit0: return QStringLiteral("u0");
        case Unit1: return QStringLiteral("u1");
        case Unit2: return QStringLiteral("u2");
        case Unit3: return QStringLiteral("u3");
        default: return QString();
        }
    }
};

class ut_DAbstractUnitFormatter : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
    TestUnitFormatter formatter;
};

TEST_F(ut_DAbstractUnitFormatter, formatAsSameUnit)
{
    qreal result = formatter.formatAs(5, TestUnitFormatter::Unit1, TestUnitFormatter::Unit1);
    EXPECT_TRUE(qFuzzyCompare(result, 5.0));
}

TEST_F(ut_DAbstractUnitFormatter, formatAsToBiggerUnit)
{
    qreal result = formatter.formatAs(100, TestUnitFormatter::Unit0, TestUnitFormatter::Unit1);
    EXPECT_TRUE(qFuzzyCompare(result, 10.0));
}

TEST_F(ut_DAbstractUnitFormatter, formatAsToSmallerUnit)
{
    qreal result = formatter.formatAs(1, TestUnitFormatter::Unit1, TestUnitFormatter::Unit0);
    EXPECT_TRUE(qFuzzyCompare(result, 10.0));
}

TEST_F(ut_DAbstractUnitFormatter, formatAsMultiStepUp)
{
    // formatAs(1000, Unit0, Unit2): 1000/10(Unit0)=100, 100/100(Unit1)=1
    qreal result = formatter.formatAs(1000, TestUnitFormatter::Unit0, TestUnitFormatter::Unit2);
    EXPECT_TRUE(qFuzzyCompare(result, 1.0));
}

TEST_F(ut_DAbstractUnitFormatter, formatAsMultiStepDown)
{
    qreal result = formatter.formatAs(1, TestUnitFormatter::Unit2, TestUnitFormatter::Unit0);
    EXPECT_TRUE(qFuzzyCompare(result, 1000.0));
}

TEST_F(ut_DAbstractUnitFormatter, formatAsMaxToMin)
{
    qreal result = formatter.formatAs(1, TestUnitFormatter::Unit3, TestUnitFormatter::Unit0);
    EXPECT_TRUE(qFuzzyCompare(result, 1000.0 * 100.0 * 10.0));
}

TEST_F(ut_DAbstractUnitFormatter, formatAsMinToMax)
{
    qreal result = formatter.formatAs(1000000, TestUnitFormatter::Unit0, TestUnitFormatter::Unit3);
    EXPECT_TRUE(qFuzzyCompare(result, 1.0));
}

TEST_F(ut_DAbstractUnitFormatter, formatAsZero)
{
    qreal result = formatter.formatAs(0, TestUnitFormatter::Unit0, TestUnitFormatter::Unit2);
    EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST_F(ut_DAbstractUnitFormatter, formatAsNegative)
{
    qreal result = formatter.formatAs(-100, TestUnitFormatter::Unit0, TestUnitFormatter::Unit1);
    EXPECT_DOUBLE_EQ(result, -10.0);
}

TEST_F(ut_DAbstractUnitFormatter, formatToBiggerUnit)
{
    // format(1000, Unit0): 1000>9 -> 100(Unit1), 100>99 -> 1(Unit2), 1<999 stop
    QPair<qreal, int> result = formatter.format(1000, TestUnitFormatter::Unit0);
    EXPECT_TRUE(qFuzzyCompare(result.first, 1.0));
    EXPECT_EQ(result.second, TestUnitFormatter::Unit2);
}

TEST_F(ut_DAbstractUnitFormatter, formatToSmallerUnit)
{
    QPair<qreal, int> result = formatter.format(0.1, TestUnitFormatter::Unit1);
    EXPECT_TRUE(qFuzzyCompare(result.first, 1.0));
    EXPECT_EQ(result.second, TestUnitFormatter::Unit0);
}

TEST_F(ut_DAbstractUnitFormatter, formatNoConversionNeeded)
{
    QPair<qreal, int> result = formatter.format(5, TestUnitFormatter::Unit1);
    EXPECT_TRUE(qFuzzyCompare(result.first, 5.0));
    EXPECT_EQ(result.second, TestUnitFormatter::Unit1);
}

TEST_F(ut_DAbstractUnitFormatter, formatAtMinUnit)
{
    QPair<qreal, int> result = formatter.format(0.001, TestUnitFormatter::Unit0);
    EXPECT_EQ(result.second, TestUnitFormatter::Unit0);
}

TEST_F(ut_DAbstractUnitFormatter, formatAtMaxUnit)
{
    QPair<qreal, int> result = formatter.format(10000, TestUnitFormatter::Unit3);
    EXPECT_EQ(result.second, TestUnitFormatter::Unit3);
}

TEST_F(ut_DAbstractUnitFormatter, formatAsUnitListBasic)
{
    QList<QPair<qreal, int>> result = formatter.formatAsUnitList(123, TestUnitFormatter::Unit0);
    ASSERT_FALSE(result.isEmpty());
}

TEST_F(ut_DAbstractUnitFormatter, formatAsUnitListZero)
{
    QList<QPair<qreal, int>> result = formatter.formatAsUnitList(0, TestUnitFormatter::Unit0);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(ut_DAbstractUnitFormatter, formatAsUnitListSingleUnit)
{
    QList<QPair<qreal, int>> result = formatter.formatAsUnitList(5, TestUnitFormatter::Unit0);
    ASSERT_EQ(result.size(), 1);
    EXPECT_TRUE(qFuzzyCompare(result[0].first, 5.0));
    EXPECT_EQ(result[0].second, TestUnitFormatter::Unit0);
}

TEST_F(ut_DAbstractUnitFormatter, formatAsUnitListMultipleUnits)
{
    QList<QPair<qreal, int>> result = formatter.formatAsUnitList(1111, TestUnitFormatter::Unit0);
    EXPECT_FALSE(result.isEmpty());
    EXPECT_GE(result.size(), 1);
}

TEST_F(ut_DAbstractUnitFormatter, formatAsUnitListAtMaxUnit)
{
    QList<QPair<qreal, int>> result = formatter.formatAsUnitList(5, TestUnitFormatter::Unit3);
    ASSERT_EQ(result.size(), 1);
    EXPECT_TRUE(qFuzzyCompare(result[0].first, 5.0));
    EXPECT_EQ(result[0].second, TestUnitFormatter::Unit3);
}

TEST_F(ut_DAbstractUnitFormatter, formatAsUnitListConvertsDown)
{
    QList<QPair<qreal, int>> result = formatter.formatAsUnitList(0.5, TestUnitFormatter::Unit1);
    EXPECT_FALSE(result.isEmpty());
    EXPECT_EQ(result.last().second, TestUnitFormatter::Unit0);
}

TEST_F(ut_DAbstractUnitFormatter, unitStr)
{
    EXPECT_EQ(formatter.unitStr(TestUnitFormatter::Unit0), QStringLiteral("u0"));
    EXPECT_EQ(formatter.unitStr(TestUnitFormatter::Unit1), QStringLiteral("u1"));
    EXPECT_EQ(formatter.unitStr(TestUnitFormatter::Unit2), QStringLiteral("u2"));
    EXPECT_EQ(formatter.unitStr(TestUnitFormatter::Unit3), QStringLiteral("u3"));
}

TEST_F(ut_DAbstractUnitFormatter, unitValueMaxDefault)
{
    EXPECT_TRUE(qFuzzyCompare(formatter.unitValueMax(TestUnitFormatter::Unit0), 9.0));
    EXPECT_TRUE(qFuzzyCompare(formatter.unitValueMax(TestUnitFormatter::Unit1), 99.0));
}

TEST_F(ut_DAbstractUnitFormatter, unitValueMinDefault)
{
    EXPECT_TRUE(qFuzzyCompare(formatter.unitValueMin(TestUnitFormatter::Unit0), 1.0));
    EXPECT_TRUE(qFuzzyCompare(formatter.unitValueMin(TestUnitFormatter::Unit1), 1.0));
}
