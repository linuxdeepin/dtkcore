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

#include "ddisksizeformatter.h"

#include <QString>

DCORE_BEGIN_NAMESPACE

DDiskSizeFormatter::DDiskSizeFormatter()
    : DAbstractUnitFormatter()
{

}

QString DDiskSizeFormatter::unitStr(int unitId) const
{
    switch (unitId)
    {
    case B:     return QStringLiteral("B");
    case K:     return QStringLiteral("KB");
    case M:     return QStringLiteral("MB");
    case G:     return QStringLiteral("GB");
    case T:     return QStringLiteral("TB");
    }

    return QString();
}

DDiskSizeFormatter DDiskSizeFormatter::rate(int rate)
{
    m_rate = rate;

    return *this;
}

DCORE_END_NAMESPACE
