/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include "dabstractunitformatter.h"

DCORE_BEGIN_NAMESPACE

/*!
 * \~chinese \class DAbstractUnitFormatter
 * \~chinese \brief DAbstractUnitFormatter 类是对拥有相同类型数据管理的接口类
 * 接口定义了最大值、最小值、转换单位和单位对应的字符串。
 *
 * \~chinese \fn DAbstractUnitFormatter::unitMax
 * \~chinese \brief 返回列表中最大的单位
 *
 * \~chinese \fn DAbstractUnitFormatter::unitMin
 * \~chinese \brief 返回列表中最小的单位
 *
 * \~chinese \fn DAbstractUnitFormatter::unitConvertRate
 * \~chinese \brief 返回当前设置的转换单位
 *
 * \~chinese \fn DAbstractUnitFormatter::unitValueMax
 * \~chinese \brief 返回列表中根据当前设置的转换单位的最大值
 *
 * \~chinese \fn DAbstractUnitFormatter::unitValueMin
 * \~chinese \brief 返回列表中根据当前设置的转换单位的最小值
 *
 * \~chinese \fn DAbstractUnitFormatter::unitStr
 * \~chinese \brief 传入id，返回列表中对应的字符串
 */

/*!
 * \~chinese \brief DAbstractUnitFormatter 的构造函数
 *
 */
DAbstractUnitFormatter::DAbstractUnitFormatter()
{

}

/*!
 * \~chinese \brief DAbstractUnitFormatter 的析构函数
 *
 */
DAbstractUnitFormatter::~DAbstractUnitFormatter()
{

}

/*!
 * \~chinese \brief 将传入的值从当前转换单位转换到目标单位上，返回转换过的值
 * 如果当前转换单位小于目标单位，值会被缩小，反之会放大，当前转换单位也会被缩小和放大，直至当前转换单位等于目标单位。
 *
 * @param value 原始数值
 * @param currentUnit 当前的转换比率
 * @param targetUnit 目标的转换比率
 * @return qreal 返回转换过的值
 */
qreal DAbstractUnitFormatter::formatAs(qreal value, int currentUnit, const int targetUnit) const
{
    while (currentUnit < targetUnit)
        value /= unitConvertRate(currentUnit++);
    while (currentUnit > targetUnit)
        value *= unitConvertRate(--currentUnit);

    return value;
}

/*!
 * \~chinese \brief 将值转换到最合适的单位上
 *
 * 如果值大于 unitMin() 或者小于 unitMax() ，会尽量保证值被转换到接近最小值的合适单位上。
 *
 * @param value 原始数值
 * @param unit 当前的转换单位
 * @return QPair<qreal, int> 转换过的数值和转化单位
 */
QPair<qreal, int> DAbstractUnitFormatter::format(const qreal value, const int unit) const
{
    // can convert to smaller unit
    if (unit > unitMin() && value < unitValueMin(unit))
        return format(value * unitConvertRate(unit - 1), unit - 1);

    // can convert to bigger unit
    if (unit < unitMax() && value > unitValueMax(unit))
        return format(value / unitConvertRate(unit), unit + 1);

    return QPair<qreal, int>(value, unit);
}

/*!
 * \~chinese \brief 是 format() ，但是包含了完整的转换数据
 *
 * @param value
 * @param unit
 * @return QList<QPair<qreal, int> >
 */
QList<QPair<qreal, int> > DAbstractUnitFormatter::formatAsUnitList(const qreal value, int unit) const
{
    if (qFuzzyIsNull(value))
        return QList<QPair<qreal, int>>();

    if (value < unitValueMin(unit) || unit == unitMin())
    {
        if (unit != unitMin())
            return formatAsUnitList(value * unitConvertRate(unit - 1), unit - 1);
        else
            return std::move(QList<QPair<qreal, int>>() << QPair<qreal, int>(value, unit));
    }

    ulong _value = ulong(value);
    QList<QPair<qreal, int>> ret = formatAsUnitList(value - _value, unit);

    while (_value && unit != unitMax())
    {
        const ulong rate = unitConvertRate(unit);
        const ulong r = _value % rate;
        if (r)
            ret.push_front(QPair<qreal, int>(r, unit));

        unit += 1;
        _value /= rate;
    }

    if (_value)
        ret.push_front(QPair<qreal, int>(_value, unit));

    return ret;
}

DCORE_END_NAMESPACE
