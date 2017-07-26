#pragma once

#include <QtCore/qglobal.h>

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


#if defined(STATIC_LIB)
#  define LIBDTKCORESHARED_EXPORT
#else
#if defined(LIBDTKCORE_LIBRARY)
#  define LIBDTKCORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBDTKCORESHARED_EXPORT Q_DECL_IMPORT
#endif
#endif
