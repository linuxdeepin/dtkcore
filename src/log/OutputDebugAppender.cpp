/*
  Copyright (c) 2010 Karl-Heinz Reichel (khreichel at googlemail dot com)

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
#include "OutputDebugAppender.h"

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

  The \a timeStamp parameter indicates the time stamp.
  The \a logLevel parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a function parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a message parameter indicates the output message.

  \sa AbstractStringAppender::format()
 */
void OutputDebugAppender::append(const QDateTime& timeStamp,
                                 Logger::LogLevel logLevel,
                                 const char* file,
                                 int line,
                                 const char* function,
                                 const QString& category,
                                 const QString& message)
{
    QString s = formattedString(timeStamp, logLevel, file, line, function, category, message);
    OutputDebugStringW((LPCWSTR) s.utf16());
}

DCORE_END_NAMESPACE
