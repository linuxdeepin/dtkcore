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

#include "ddisksizeformatter.h"

#include <QString>

DCORE_BEGIN_NAMESPACE

/*!
  \class Dtk::Core::DDiskSizeFormatter
  \inmodule dtkcore
  \brief DDiskSizeFormatter 是用来获取磁盘容量单位的类, 通过枚举值.
  
  获取不同类型磁盘容量的单位
 */

/*!
  \enum Dtk::Core::DDiskSizeFormatter::DiskUnits
  磁盘容量单位的枚举
  \value B
  字节
  \value K
  千字节
  \value M
  兆字节
  \value G
  吉字节
  \value T
  太字节
 */

/*!
  \reimp
  \fn int DDiskSizeFormatter::unitMax() const
  \brief 返回最大磁盘容量单位的枚举
 */

/*!
  \reimp
  \fn int DDiskSizeFormatter::unitMin() const
  \brief 返回最小磁盘容量单位的枚举
 */

/*!
  \reimp
  \fn uint DDiskSizeFormatter::unitConvertRate(int unitId) const
  \brief 返回当前的单位转换比率
 */

/*!
  \brief DDiskSizeFormatter的构造函数
  
 */
DDiskSizeFormatter::DDiskSizeFormatter()
    : DAbstractUnitFormatter()
{

}

/*!
  \brief 根据枚举返回对应单位的字符串
  
  \a unitId DDiskSizeFormatter::DiskUnits 的枚举值
  \return QString 对应单位的字符串
 */
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

/*!
  \brief 设置当前的单位转换比率
  
  \a rate 转换比率
  \return DDiskSizeFormatter 返回 DDiskSizeFormatter 对象
 */
DDiskSizeFormatter DDiskSizeFormatter::rate(int rate)
{
    m_rate = rate;

    return *this;
}

DCORE_END_NAMESPACE
