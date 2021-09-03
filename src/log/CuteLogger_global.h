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

#ifndef CUTELOGGER_GLOBAL_H
#define CUTELOGGER_GLOBAL_H

#include <dtkcore_global.h>

#if defined(CUTELOGGER_LIBRARY)
#  define CUTELOGGERSHARED_EXPORT Q_DECL_EXPORT
#else
#if defined(Q_OS_WIN32)
#  define CUTELOGGERSHARED_EXPORT
#else
#  define CUTELOGGERSHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

#endif // CUTELOGGER_GLOBAL_H
