// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// Local
#include "win32/OutputDebugAppender.h"

#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

// STL
#include <windows.h>

DCORE_BEGIN_NAMESPACE

/*!
  \class Dtk::Core::OutputDebugAppender
  \inmodule dtkcore
  
  \brief Appender that writes the log records to the Microsoft Debug Log.
 */

/*!
  \brief Writes the log record to the windows debug log.
  \reimp

  \brief Writes the log record to the windows debug log.

  The \a time parameter indicates the time stamp.
  The \a level parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a func parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a msg parameter indicates the output message.

  \sa AbstractStringAppender::format()
 */
void OutputDebugAppender::append(const QDateTime &time,
                                 Logger::LogLevel level,
                                 const char *file,
                                 int line,
                                 const char *func,
                                 const QString &category,
                                 const QString &msg)
{
    auto msvclogger = spdlog::get("msvc");
    if (!msvclogger)
        msvclogger = spdlog::create<spdlog::sinks::msvc_sink_mt>("msvc", true);

    Q_ASSERT(msvclogger);

    msvclogger->set_level(spdlog::level::level_enum(detailsLevel()));
    const auto &formatted = formattedString(time, level, file, line, func, category, msg);
    msvclogger->log(spdlog::level::level_enum(level), formatted.toStdString());
    msvclogger->flush();
}

DCORE_END_NAMESPACE
