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

#include "dtimeunitformatter.h"

#include <QString>

DCORE_BEGIN_NAMESPACE

DTimeUnitFormatter::DTimeUnitFormatter()
    : DAbstractUnitFormatter()
{

}

uint DTimeUnitFormatter::unitConvertRate(int unitId) const
{
    switch (unitId)
    {
    case Seconds:   return 60;
    case Minute:    return 60;
    case Hour:      return 24;
    default:;
    }

    return 0;
}

QString DTimeUnitFormatter::unitStr(int unitId) const
{
    switch (unitId)
    {
    case Seconds:   return QStringLiteral("s");
    case Minute:    return QStringLiteral("m");
    case Hour:      return QStringLiteral("h");
    case Day:       return QStringLiteral("d");
    default:;
    }

    return QString();
}

DCORE_END_NAMESPACE
