// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// Local

#include "ConsoleAppender.h"

#include <spdlog/spdlog.h>
#include <spdlog/spdlog-inl.h>
#include <spdlog/details/registry.h>
#include <spdlog/details/registry-inl.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// STL
#include <iostream>
extern "C" {
#include <unistd.h>
}

DCORE_BEGIN_NAMESPACE

/*!
@~english
  @class Dtk::Core::ConsoleAppender
  @ingroup dlog

  @brief ConsoleAppender is the simple appender that writes the log records to the std::cerr output stream.

  ConsoleAppender uses "[%{type:-7}] <%{function}> %{message}\n" as a default output format. It is similar to the
  AbstractStringAppender but doesn't show a time.

  You can modify ConsoleAppender output format without modifying your code by using \c QT_MESSAGE_PATTERN environment
  variable. If you need your application to ignore this environment variable you can call
  ConsoleAppender::ignoreEnvironmentPattern(true)
 */


ConsoleAppender::ConsoleAppender()
    : AbstractStringAppender()
    ,m_ignoreEnvPattern(false)
{
    if (!spdlog::get("console")) {
        auto clogger = spdlog::stdout_color_mt("console");
        clogger->set_level(spdlog::level::level_enum(detailsLevel()));
    }
}


QString ConsoleAppender::format() const
{
    const QString envPattern = QString::fromLocal8Bit(qgetenv("QT_MESSAGE_PATTERN"));
    return (m_ignoreEnvPattern || envPattern.isEmpty()) ? AbstractStringAppender::format() : (envPattern + "\n");
}


void ConsoleAppender::ignoreEnvironmentPattern(bool ignore)
{
    m_ignoreEnvPattern = ignore;
}

/*!
@~english
  \brief Writes the log record to the std::cerr stream.
  \reimp

  The \a time parameter indicates the time stamp.
  The \a level parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a func parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a msg parameter indicates the output message.

  \sa AbstractStringAppender::format()
 */
void ConsoleAppender::append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                             const char *func, const QString &category, const QString &msg)
{
    auto clogger = spdlog::get("console");
    Q_ASSERT(clogger);
    clogger->set_level(spdlog::level::level_enum(detailsLevel()));

    const auto &formatted = formattedString(time, level, file, line, func, category, msg, isatty(STDOUT_FILENO));
    clogger->log(spdlog::level::level_enum(level), formatted.toStdString());
}

DCORE_END_NAMESPACE
