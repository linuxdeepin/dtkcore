// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtimeunitformatter.h"

#include <QString>

DCORE_BEGIN_NAMESPACE

/*!
  \class Dtk::Core::DTimeUnitFormatter
  \inmodule dtkcore
  
  \brief DTimeUnitFormatter是用来获取时间单位的类, 通过枚举值.

  获取不同类型时间单位的进制
 */

/*!
  \enum Dtk::Core::DTimeUnitFormatter::TimeUnits
  时间单位的枚举
  \value Seconds
  返回分钟单位的进制
  \value Minute
  返回秒单位的进制
  \value Hour
  返回小时单位的进制
  \value Day
  返回天单位的进制
 */

/*!
  \fn int DTimeUnitFormatter::unitMax() const
  \brief 返回最大时间单位的枚举
 */

/*!
  \fn int DTimeUnitFormatter::unitMin() const
  \brief 返回最小时间单位的枚举
 */

/*!
  \brief DTimeUnitFormatter的构造函数
  
 */
DTimeUnitFormatter::DTimeUnitFormatter()
    : DAbstractUnitFormatter()
{

}

/*!
  \brief 根据枚举返回对应的单位进制
  
  \a unitId DTimeUnitFormatter::TimeUnits 的枚举值
  \return uint 对应的单位进制
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
  \brief 根据枚举返回对应单位的缩写
  
  \a unitId DTimeUnitFormatter::TimeUnits 的枚举值
  \return QString 对应单位的缩写
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
