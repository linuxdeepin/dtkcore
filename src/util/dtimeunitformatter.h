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

#ifndef DTIMEUNITFORMATTER_H
#define DTIMEUNITFORMATTER_H

#include <dtkcore_global.h>
#include <dabstractunitformatter.h>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DTimeUnitFormatter : public DAbstractUnitFormatter
{
public:
    DTimeUnitFormatter();

    enum TimeUnits
    {
        Seconds,
        Minute,
        Hour,
        Day,
    };

    QString unitStr(int unitId) const override;

protected:
    int unitMax() const override { return Day; }
    int unitMin() const override { return Seconds; }
    uint unitConvertRate(int unitId) const override;
};

DCORE_END_NAMESPACE

#endif // DTIMEUNITFORMATTER_H
