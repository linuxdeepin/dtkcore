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

#include <QtCore/qglobal.h>
#include <dtkcore_config.h>

#define DTK_NAMESPACE Dtk

#if !defined(DTK_NAMESPACE)
#   define DTK_BEGIN_NAMESPACE
#   define DTK_END_NAMESPACE
#   define DTK_USE_NAMESPACE
#else
#   define DTK_BEGIN_NAMESPACE namespace DTK_NAMESPACE {
#   define DTK_END_NAMESPACE }
#   define DTK_USE_NAMESPACE using namespace DTK_NAMESPACE;
#endif

#define DCORE_NAMESPACE Core
#define DTK_CORE_NAMESPACE DTK_NAMESPACE::DCORE_NAMESPACE

#if !defined(DCORE_NAMESPACE)
#   define DCORE_BEGIN_NAMESPACE
#   define DCORE_END_NAMESPACE
#   define DCORE_USE_NAMESPACE
#else
#   define DCORE_BEGIN_NAMESPACE namespace DTK_NAMESPACE { namespace DCORE_NAMESPACE {
#   define DCORE_END_NAMESPACE }}
#   define DCORE_USE_NAMESPACE using namespace DTK_CORE_NAMESPACE;
#endif


#if defined(DTK_STATIC_LIB)
#  define LIBDTKCORESHARED_EXPORT
#else
#if defined(LIBDTKCORE_LIBRARY)
#  define LIBDTKCORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBDTKCORESHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

#ifdef D_DEPRECATED_CHECK
#define D_DECL_DEPRECATED_X(text) Q_DECL_HIDDEN
#define D_DECL_DEPRECATED Q_DECL_HIDDEN
#else
#define D_DECL_DEPRECATED Q_DECL_DEPRECATED
#define D_DECL_DEPRECATED_X Q_DECL_DEPRECATED_X
#endif

#define DTK_VERSION_CHECK(major, minor, patch, build) ((major<<24)|(minor<<16)|(patch<<8)|build)
#define DTK_VERSION DTK_VERSION_CHECK(DTK_VERSION_MAJOR, DTK_VERSION_MINOR, DTK_VERSION_PATCH, DTK_VERSION_BUILD)

extern "C" {
int dtkVersion();
const char *dtkVersionString();
}
