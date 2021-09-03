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

#ifndef DISKSIZEFORMATTER_H
#define DISKSIZEFORMATTER_H

#include <dabstractunitformatter.h>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DDiskSizeFormatter : public DAbstractUnitFormatter
{
public:
    DDiskSizeFormatter();

    enum DiskUnits
    {
        B,
        K,
        M,
        G,
        T,
    };

    QString unitStr(int unitId) const override;

    DDiskSizeFormatter rate(int rate);

protected:
    int unitMin() const override { return B; }
    int unitMax() const override { return T; }
    uint unitConvertRate(int unitId) const override { Q_UNUSED(unitId); return m_rate; }

private:
    int m_rate = 1000;
};

DCORE_END_NAMESPACE

#endif // DISKSIZEFORMATTER_H
