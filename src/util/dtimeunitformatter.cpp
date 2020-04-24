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

#include "dtimeunitformatter.h"

#include <QString>

DCORE_BEGIN_NAMESPACE

/*!
 * \~chinese \class DTimeUnitFormatter
 *
 * \~chinese \brief DTimeUnitFormatter是用来获取时间单位的类, 通过枚举值
 * 获取不同类型时间单位的进制
 *
 * \~chinese \enum DTimeUnitFormatter::TimeUnits 时间单位的枚举
 * \~chinese \var DTimeUnitFormatter::TimeUnits DTimeUnitFormatter::Seconds
 * \~chinese \brief 返回分钟单位的进制
 * \~chinese \var DTimeUnitFormatter::TimeUnits DTimeUnitFormatter::Minute
 * \~chinese \brief 返回秒单位的进制
 * \~chinese \var DTimeUnitFormatter::TimeUnits DTimeUnitFormatter::Hour
 * \~chinese \brief 返回小时单位的进制
 * \~chinese \var DTimeUnitFormatter::TimeUnits DTimeUnitFormatter::Day
 * \~chinese \brief 返回天单位的进制
 *
 * \~chinese \fn DTimeUnitFormatter::unitMax
 * \~chinese \brief 返回最大时间单位的枚举
 *
 * \~chinese \fn DTimeUnitFormatter::unitMin
 * \~chinese \brief 返回最小时间单位的枚举
 */

/*!
 * \~chinese \brief DTimeUnitFormatter的构造函数
 *
 */
DTimeUnitFormatter::DTimeUnitFormatter()
    : DAbstractUnitFormatter()
{

}

/*!
 * \~chinese \brief 根据枚举返回对应的单位进制
 *
 * @param unitId DTimeUnitFormatter::TimeUnits 的枚举值
 * @return uint 对应的单位进制
 */
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

/*!
 * \~chinese \brief 根据枚举返回对应单位的缩写
 *
 * @param unitId DTimeUnitFormatter::TimeUnits 的枚举值
 * @return QString 对应单位的缩写
 */
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
