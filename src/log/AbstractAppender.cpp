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

 // Local
#include "AbstractAppender.h"

// Qt
#include <QMutexLocker>

DCORE_BEGIN_NAMESPACE

/*!
  \class Dtk::Core::AbstractAppender
  \inmodule dtkcore
  
  \brief The AbstractAppender class provides an abstract base class for writing a log entries.
  
  The AbstractAppender class is the base interface class for all log appenders that could be used with Logger.
  
  AbstractAppender provides a common implementation for the thread safe, mutex-protected logging of application
  messages, such as ConsoleAppender, FileAppender or something else. AbstractAppender is abstract and can not be
  instantiated, but you can use any of its subclasses or create a custom log appender at your choice.
  
  Appenders are the logical devices that is aimed to be attached to Logger object by calling
  Logger::registerAppender(). On each log record call from the application Logger object sequentially calls write()
  function on all the appenders registered in it.
  
  You can subclass AbstractAppender to implement a logging target of any kind you like. It may be the external logging
  subsystem (for example, syslog in *nix), XML file, SQL database entries, D-Bus messages or anything else you can
  imagine.
  
  For the simple non-structured plain text logging (for example, to a plain text file or to the console output) you may
  like to subclass the AbstractStringAppender instead of AbstractAppender, which will give you a more convenient way to
  control the format of the log output.
  
  \sa AbstractStringAppender
  \sa Logger::registerAppender()
 */


/*!
    \brief Constructs a AbstractAppender object.
 */
AbstractAppender::AbstractAppender()
  : m_detailsLevel(Logger::Debug)
{}

/*!
    \brief Destructs the AbstractAppender object.
 */
AbstractAppender::~AbstractAppender()
{}

/*!
  \brief Returns the current details level of appender.

  Log records with a log level lower than a current detailsLevel() will be silently ignored by appender and would not
  be sent to its append() function.
  
  It provides additional logging flexibility, allowing you to set the different severity levels for different types
  of logs.
  
  \note This function is thread safe.

  \return The log level.
  
  \sa setDetailsLevel()
  \sa Logger::LogLevel
 */
Logger::LogLevel AbstractAppender::detailsLevel() const
{
  QMutexLocker locker(&m_detailsLevelMutex);
  return m_detailsLevel;
}

/*!
  \brief Sets the current details level of appender.

  Default details \a level is Logger::Debug
  
  \note This function is thread safe.
  
  \sa detailsLevel()
  \sa Logger::LogLevel
 */
void AbstractAppender::setDetailsLevel(Logger::LogLevel level)
{
  QMutexLocker locker(&m_detailsLevelMutex);
  m_detailsLevel = level;
}

/*!
  \brief Sets the current details \a level of appender.

  This function is provided for convenience, it behaves like an above function.
  
  \sa detailsLevel()
  \sa Logger::LogLevel
 */
void AbstractAppender::setDetailsLevel(const QString& level)
{
  setDetailsLevel(Logger::levelFromString(level));
}

/*!
  \brief Tries to write the log record to this logger.

  This is the function called by Logger object to write a log \a message to the appender.

  The \a timeStamp parameter indicates the time stamp.
  The \a logLevel parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a function parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a message parameter indicates the output message.
  
  \note This function is thread safe.
  
  \sa Logger::write()
  \sa detailsLevel()
 */
void AbstractAppender::write(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                             const char* function, const QString& category, const QString& message)
{
  if (logLevel >= detailsLevel())
  {
    QMutexLocker locker(&m_writeMutex);
    append(timeStamp, logLevel, file, line, function, category, message);
  }
}


/*!
  \fn virtual void AbstractAppender::append(const QDateTime &timeStamp, Logger::LogLevel logLevel, const char *file, int line,
                        const char *function, const QString &category, const QString &message) = 0
  
  \brief Writes the log record to the logger instance
  
  This function is called every time when user tries to write a message to this AbstractAppender instance using
  the write() function. Write function works as proxy and transfers only the messages with log level more or equal
  to the current logLevel().
  
  Overload this function when you are implementing a custom appender.

  The \a timeStamp parameter indicates the time stamp.
  The \a logLevel parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a function parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a message parameter indicates the output message.
  
  \note This function is not needed to be thread safe because it is never called directly by Logger object. The
  write() function works as a proxy and protects this function from concurrent access.
  
  \sa Logger::write()
 */

DCORE_END_NAMESPACE
