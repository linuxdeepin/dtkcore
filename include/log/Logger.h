// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>

#include "dloggerdefs.h"

DCORE_BEGIN_NAMESPACE

class AbstractAppender;
class LoggerPrivate;
class LIBDTKCORESHARED_EXPORT Logger
{
    Q_DISABLE_COPY(Logger)
public:
    //!@~english the log levels
    enum LogLevel {
        Trace,   //!<@~english Trace level. Can be used for mostly unneeded records used for internal code tracing.
        Debug,   //!<@~english Debug level.for the debugging of the software.
        Info,    //!<@~english Info level. Can be used for informational records, which may be interesting for not only developers.
        Warning, //!<@~english Warning. May be used to log some non-fatal warnings detected by your application.
        Error,   //!<@~english May be used for a big problems making your application work wrong but not crashing.
        Fatal    //!<@~english Fatal. Used for unrecoverable errors, crashes the application (abort) right after the log record is written.
    };

    Logger();
    Logger(const QString &defaultCategory);
    ~Logger();

    static Logger *globalInstance();

    static QString levelToString(LogLevel level);
    static LogLevel levelFromString(const QString &str);

    void registerAppender(AbstractAppender *appender);
    void registerCategoryAppender(const QString &category, AbstractAppender *appender);

    void logToGlobalInstance(const QString &category, bool logToGlobal = false);

    void setDefaultCategory(const QString &category);
    QString defaultCategory() const;

    void write(const QDateTime &time, LogLevel level, const char *file, int line,
               const char *func, const char *category, const QString &msg);
    void write(LogLevel level, const char *file, int line,
               const char *func, const char *category, const QString &msg);
    QDebug write(LogLevel level, const char *file, int line,
                 const char *func, const char *category);
    void writeAssert(const char *file, int line,
                     const char *func, const char *condition);

private:
    void write(const QDateTime &time, LogLevel level, const char *file, int line,
               const char *func, const char *category,
               const QString &msg, bool fromLocalInstance);
    Q_DECLARE_PRIVATE(Logger)
    LoggerPrivate *d_ptr;
};


class LIBDTKCORESHARED_EXPORT CuteMessageLogger
{
    Q_DISABLE_COPY(CuteMessageLogger)

public:
    Q_DECL_CONSTEXPR CuteMessageLogger(Logger *l, Logger::LogLevel level,
                                       const char *file, int line, const char *func)
        : m_l(l),
          m_level(level),
          m_file(file),
          m_line(line),
          m_function(func),
          m_category(nullptr)
    {}

    Q_DECL_CONSTEXPR CuteMessageLogger(Logger *l, Logger::LogLevel level, const char *file,
                                       int line, const char *func, const char *category)
        : m_l(l),
          m_level(level),
          m_file(file),
          m_line(line),
          m_function(func),
          m_category(category)
    {}

    void write(const char *msg, ...) const
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    #if defined(Q_CC_MINGW) && !defined(Q_CC_CLANG)
        __attribute__((format(gnu_printf, 2, 3)));
    #else
        __attribute__((format(printf, 2, 3)));
    #endif
#endif

    void write(const QString &msg) const;
    QDebug write() const;

private:
    Logger *m_l;
    Logger::LogLevel m_level;
    const char *m_file;
    int m_line;
    const char *m_function;
    const char *m_category;
};

class LIBDTKCORESHARED_EXPORT LoggerTimingHelper
{
    Q_DISABLE_COPY(LoggerTimingHelper)
public:
    inline explicit LoggerTimingHelper(Logger *l, Logger::LogLevel level,
                                       const char *file, int line, const char *func)
        : m_logger(l),
          m_logLevel(level),
          m_file(file),
          m_line(line),
          m_function(func)
    {}

    void start(const char *msg, ...)
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    #if defined(Q_CC_MINGW) && !defined(Q_CC_CLANG)
        __attribute__((format(gnu_printf, 2, 3)));
    #else
        __attribute__((format(printf, 2, 3)));
    #endif
#endif

    void start(const QString &msg = QString());

    ~LoggerTimingHelper();

private:
    Logger *m_logger;
    QElapsedTimer m_time;
    Logger::LogLevel m_logLevel;
    const char *m_file;
    int m_line;
    const char *m_function;
    QString m_block;
};

DCORE_END_NAMESPACE
#endif // LOGGER_H
