/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dabstractunitformatter.h"

DCORE_BEGIN_NAMESPACE

DAbstractUnitFormatter::DAbstractUnitFormatter()
{

}

DAbstractUnitFormatter::~DAbstractUnitFormatter()
{

}

qreal DAbstractUnitFormatter::formatAs(qreal value, int currentUnit, const int targetUnit) const
{
    while (currentUnit < targetUnit)
        value /= unitConvertRate(currentUnit++);
    while (currentUnit > targetUnit)
        value *= unitConvertRate(--currentUnit);

    return value;
}

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
