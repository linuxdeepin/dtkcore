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

#pragma once

#include <QDir>

#include <dtkcore_global.h>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DPathBuf
{
public:
    DPathBuf();
    DPathBuf(const QString &path);

    /*!
     * \brief join path with operator /
     * \param p is subpath
     * \return a new DPathBuf with subpath p
     */
    DPathBuf operator/(const QString &p) const
    {
        return DPathBuf(m_path + "/" + p);
    }

    /*!
     * \brief join path to self with operator /=
     * \param p is subpath to join
     * \return self object
     */
    DPathBuf &operator/=(const QString &p)
    {
        return join(p);
    }

    /*!
     * \brief join path with operator /
     * \param p is subpath
     * \return a new DPathBuf with subpath p
     * \sa Dtk::Core::DPathBuf::operator/(const QString &p) const
     */
    DPathBuf operator/(const char *p) const
    {
        return operator /(QString(p));
    }

    /*!
     * \brief join path to self with operator /=
     * \param p is subpath to join
     * \return self object
     * \sa Dtk::Core::DPathBuf::operator/=(const QString &p)
     */
    DPathBuf &operator/=(const char *p)
    {
        return operator /=(QString(p));
    }

    /*!
     * \brief join add subpath p to self
     * \param p is subpath to join
     * \return slef object with subpath joined
     */
    DPathBuf &join(const QString &p)
    {
        m_path += "/" + p;
        m_path = QDir(m_path).absolutePath();
        return *this;
    }

    /*!
     * \brief toString export native separators format string.
     * \return string with native separators
     */
    QString toString() const
    {
        return QDir::toNativeSeparators(m_path);
    }

private:
    QString m_path;
};

DCORE_END_NAMESPACE
