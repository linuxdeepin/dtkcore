// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DLOGGER_DEFINE_H
#define DLOGGER_DEFINE_H

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class Logger;
class CuteMessageLogger;
class LoggerTimingHelper;
LIBDTKCORESHARED_EXPORT Logger *loggerInstance();
#define logger loggerInstance()

#define dTrace   CuteMessageLogger(loggerInstance(), Logger::Trace,   __FILE__, __LINE__, Q_FUNC_INFO).write
#define dDebug   CuteMessageLogger(loggerInstance(), Logger::Debug,   __FILE__, __LINE__, Q_FUNC_INFO).write
#define dInfo    CuteMessageLogger(loggerInstance(), Logger::Info,    __FILE__, __LINE__, Q_FUNC_INFO).write
#define dWarning CuteMessageLogger(loggerInstance(), Logger::Warning, __FILE__, __LINE__, Q_FUNC_INFO).write
#define dError   CuteMessageLogger(loggerInstance(), Logger::Error,   __FILE__, __LINE__, Q_FUNC_INFO).write
#define dFatal   CuteMessageLogger(loggerInstance(), Logger::Fatal,   __FILE__, __LINE__, Q_FUNC_INFO).write

#define dCDebug(category)   CuteMessageLogger(loggerInstance(), Logger::Debug,   __FILE__, __LINE__, Q_FUNC_INFO, category).write()
#define dCInfo(category)    CuteMessageLogger(loggerInstance(), Logger::Info,    __FILE__, __LINE__, Q_FUNC_INFO, category).write()
#define dCWarning(category) CuteMessageLogger(loggerInstance(), Logger::Warning, __FILE__, __LINE__, Q_FUNC_INFO, category).write()
#define dCError(category)   CuteMessageLogger(loggerInstance(), Logger::Error,   __FILE__, __LINE__, Q_FUNC_INFO, category).write()
#define dCFatal(category)   CuteMessageLogger(loggerInstance(), Logger::Fatal,   __FILE__, __LINE__, Q_FUNC_INFO, category).write()

#define dTraceTime  LoggerTimingHelper loggerTimingHelper(loggerInstance(), Logger::Trace, __FILE__, __LINE__, Q_FUNC_INFO); loggerTimingHelper.start
#define dDebugTime  LoggerTimingHelper loggerTimingHelper(loggerInstance(), Logger::Debug, __FILE__, __LINE__, Q_FUNC_INFO); loggerTimingHelper.start
#define dInfoTime   LoggerTimingHelper loggerTimingHelper(loggerInstance(), Logger::Info,  __FILE__, __LINE__, Q_FUNC_INFO); loggerTimingHelper.start

#define dAssert(cond)        ((!(cond)) ? loggerInstance()->writeAssert(__FILE__, __LINE__, Q_FUNC_INFO, #cond) : qt_noop())
#define dAssertX(cond, msg)  ((!(cond)) ? loggerInstance()->writeAssert(__FILE__, __LINE__, Q_FUNC_INFO, msg) : qt_noop())

#define dCategory(category) \
    private:\
    Logger* loggerInstance()\
    {\
        static Logger customLoggerInstance(category);\
        return &customLoggerInstance;\
    }\

#define dGlobalCategory(category) \
    private:\
    Logger* loggerInstance()\
    {\
        static Logger customLoggerInstance(category);\
        customLoggerInstance.logToGlobalInstance(category, true);\
        return &customLoggerInstance;\
    }\

DCORE_END_NAMESPACE

#endif // DLOGGER_DEFINE_H
