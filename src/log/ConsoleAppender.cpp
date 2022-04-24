/*
  Copyright (c) 2010 Boris Moiseev (cyberbobs at gmail dot com)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1
  as published by the Free Software Foundation and appearing in the file
  LICENSE.LGPL included in the packaging of this file.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
*/
// Local
#include "ConsoleAppender.h"

// STL
#include <iostream>

DCORE_BEGIN_NAMESPACE

/*!
  \class Dtk::Core::ConsoleAppender
  \inmodule dtkcore
  
  \brief ConsoleAppender is the simple appender that writes the log records to the std::cerr output stream.
  
  ConsoleAppender uses "[%{type:-7}] <%{function}> %{message}\n" as a default output format. It is similar to the
  AbstractStringAppender but doesn't show a timestamp.
  
  You can modify ConsoleAppender output format without modifying your code by using \c QT_MESSAGE_PATTERN environment
  variable. If you need your application to ignore this environment variable you can call
  ConsoleAppender::ignoreEnvironmentPattern(true)
 */


ConsoleAppender::ConsoleAppender()
  : AbstractStringAppender(),
    m_ignoreEnvPattern(false)
{
    setFormat(CUTELOGGER_DEFAULT_LOG_FORMAT);
}

QString ConsoleAppender::format() const
{
    //! 写的时候在 AbstractStringAppender::formattedString 里面再次被调用
    static QString logFormat;
    if (!logFormat.isEmpty()) {
        return logFormat;
    }
    // 应用自己设置的优先级最高
    QString fmt(AbstractStringAppender::format());
    // 如果用户没设置 DLogManager::setLogFormat,
    // 在 DLogManager::initRollingFileAppender 中
    // AbstractStringAppender::m_format 就被覆写为默认值了
    if (!fmt.isEmpty() && fmt != CUTELOGGER_DEFAULT_LOG_FORMAT) {
        logFormat = fmt;
        return fmt;
    }
    // 其次是环境变量里面的
    if (!m_ignoreEnvPattern) {
        fmt = QString::fromLocal8Bit(qgetenv("DTK_MESSAGE_PATTERN"));
        if (!fmt.isEmpty()) {
            logFormat = fmt;
            return fmt;
        }
    }
    logFormat = CUTELOGGER_DEFAULT_LOG_FORMAT;
    return logFormat;
}


void ConsoleAppender::ignoreEnvironmentPattern(bool ignore)
{
  m_ignoreEnvPattern = ignore;
}

/*!
  \brief Writes the log record to the std::cerr stream.
  \reimp

  The \a timeStamp parameter indicates the time stamp.
  The \a logLevel parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a function parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a message parameter indicates the output message.

  \sa AbstractStringAppender::format()
 */
void ConsoleAppender::append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                             const char* function, const QString& category, const QString& message)
{
  std::cerr << qPrintable(formattedString(timeStamp, logLevel, file, line, function, category, message));
}

DCORE_END_NAMESPACE
