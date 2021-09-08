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
#include "FileAppender.h"

// STL
#include <iostream>

DCORE_BEGIN_NAMESPACE

/*!
  \class Dtk::Core::FileAppender
  \inmodule dtkcore
  
  \brief Simple appender that writes the log records to the plain text file.
 */


/*!
    \brief Constructs the new file appender assigned to file with the given \a fileName.
 */
FileAppender::FileAppender(const QString& fileName)
{
  setFileName(fileName);
}


FileAppender::~FileAppender()
{
  closeFile();
}

/*!
  \brief Returns the name set by setFileName() or to the FileAppender constructor.

  \sa setFileName()
 */
QString FileAppender::fileName() const
{
  QMutexLocker locker(&m_logFileMutex);
  return m_logFile.fileName();
}

/*!
  \brief Sets the \a s name of the file. The name can have no path, a relative path, or an absolute path.

  \sa fileName()
 */
void FileAppender::setFileName(const QString& s)
{
  QMutexLocker locker(&m_logFileMutex);
  if (m_logFile.isOpen())
    m_logFile.close();

  m_logFile.setFileName(s);
}

qint64 FileAppender::size() const
{
    return m_logFile.size();
}


bool FileAppender::openFile()
{
  bool isOpen = m_logFile.isOpen();
  if (!isOpen)
  {
    isOpen = m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    if (isOpen)
      m_logStream.setDevice(&m_logFile);
    else
      std::cerr << "<FileAppender::append> Cannot open the log file " << qPrintable(m_logFile.fileName()) << std::endl;
  }
  return isOpen;
}

/*!
  \brief Write the log record to the file.
  \reimp

  The \a timeStamp parameter indicates the time stamp.
  The \a logLevel parameter describes the LogLevel.
  The \a file parameter is the current file name.
  The \a line parameter indicates the number of lines to output.
  The \a function parameter indicates the function name to output.
  The \a category parameter indicates the log category.
  The \a message parameter indicates the output message.

  \sa fileName()
  \sa AbstractStringAppender::format()
 */
void FileAppender::append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                          const char* function, const QString& category, const QString& message)
{
  QMutexLocker locker(&m_logFileMutex);

  if (openFile())
  {
    m_logStream << formattedString(timeStamp, logLevel, file, line, function, category, message);
    m_logStream.flush();
    m_logFile.flush();
  }
}


void FileAppender::closeFile()
{
  QMutexLocker locker(&m_logFileMutex);
  m_logFile.close();
}

DCORE_END_NAMESPACE
