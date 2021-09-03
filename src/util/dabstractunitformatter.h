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

#ifndef DABSTRACTUNITFORMATTER_H
#define DABSTRACTUNITFORMATTER_H

#include <dtkcore_global.h>

#include <QPair>
#include <QList>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DAbstractUnitFormatter
{
public:
    DAbstractUnitFormatter();
    ~DAbstractUnitFormatter();

protected:
    virtual int unitMax() const = 0;
    virtual int unitMin() const = 0;
    virtual uint unitConvertRate(int unitId) const = 0;
    virtual qreal unitValueMax(int unitId) const { return unitConvertRate(unitId) - 1; }
    virtual qreal unitValueMin(int unitId) const { Q_UNUSED(unitId); return 1; }
    virtual QString unitStr(int unitId) const = 0;

public:
    qreal formatAs(qreal value, int currentUnit, const int targetUnit) const;
    QPair<qreal, int> format(const qreal value, const int unit) const;
    QList<QPair<qreal, int>> formatAsUnitList(const qreal value, int unit) const;
};

DCORE_END_NAMESPACE

#endif // DABSTRACTUNITFORMATTER_H
