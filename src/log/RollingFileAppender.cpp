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

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

#include "RollingFileAppender.h"

DCORE_BEGIN_NAMESPACE

RollingFileAppender::RollingFileAppender(const QString& fileName)
  : FileAppender(fileName),
    m_logFilesLimit(0),
    m_logSizeLimit(1024*1024*20)
{}

void RollingFileAppender::append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
    const char* function, const QString& category, const QString& message)
{

  if (!m_rollOverTime.isNull() && QDateTime::currentDateTime() > m_rollOverTime)
    rollOver();

  if (size()> m_logSizeLimit)
    rollOver();

   FileAppender::append(timeStamp, logLevel, file, line , function, category, message);
}


RollingFileAppender::DatePattern RollingFileAppender::datePattern() const
{
  QMutexLocker locker(&m_rollingMutex);
  return m_frequency;
}


QString RollingFileAppender::datePatternString() const
{
  QMutexLocker locker(&m_rollingMutex);
  return m_datePatternString;
}


void RollingFileAppender::setDatePattern(DatePattern datePattern)
{
  setDatePatternString(QLatin1String("'.'yyyy-MM-dd-hh-mm-ss-zzz"));

  QMutexLocker locker(&m_rollingMutex);
  m_frequency = datePattern;

  computeRollOverTime();
}


void RollingFileAppender::setDatePattern(const QString& datePattern)
{
  setDatePatternString(datePattern);
  computeFrequency();

  computeRollOverTime();
}


void RollingFileAppender::setDatePatternString(const QString& datePatternString)
{
  QMutexLocker locker(&m_rollingMutex);
  m_datePatternString = datePatternString;
}


void RollingFileAppender::computeFrequency()
{
  QMutexLocker locker(&m_rollingMutex);

  const QDateTime startTime(QDate(1999, 1, 1), QTime(0, 0));
  const QString startString = startTime.toString(m_datePatternString);

  if (startString != startTime.addSecs(60).toString(m_datePatternString))
    m_frequency = MinutelyRollover;
  else if (startString != startTime.addSecs(60 * 60).toString(m_datePatternString))
    m_frequency = HourlyRollover;
  else if (startString != startTime.addSecs(60 * 60 * 12).toString(m_datePatternString))
    m_frequency = HalfDailyRollover;
  else if (startString != startTime.addDays(1).toString(m_datePatternString))
    m_frequency = DailyRollover;
  else if (startString != startTime.addDays(7).toString(m_datePatternString))
    m_frequency = WeeklyRollover;
  else if (startString != startTime.addMonths(1).toString(m_datePatternString))
    m_frequency = MonthlyRollover;
  else
  {
    Q_ASSERT_X(false, "DailyRollingFileAppender::computeFrequency", "The pattern '%1' does not specify a frequency");
    return;
  }
}


void RollingFileAppender::removeOldFiles()
{
  if (m_logFilesLimit <= 1)
    return;

  QFileInfo fileInfo(fileName());
  QDir logDirectory(fileInfo.absoluteDir());
  logDirectory.setFilter(QDir::Files);
  logDirectory.setNameFilters(QStringList() << fileInfo.fileName() + "*");
  QFileInfoList logFiles = logDirectory.entryInfoList();

  QMap<QDateTime, QString> fileDates;
  for (int i = 0; i < logFiles.length(); ++i)
  {
    QString name = logFiles[i].fileName();
    QString suffix = name.mid(name.indexOf(fileInfo.fileName()) + fileInfo.fileName().length());
    QDateTime fileDateTime = QDateTime::fromString(suffix, datePatternString());

    if (fileDateTime.isValid())
      fileDates.insert(fileDateTime, logFiles[i].absoluteFilePath());
  }

  QList<QString> fileDateNames = fileDates.values();
  for (int i = 0; i < fileDateNames.length() - m_logFilesLimit + 1; ++i)
    QFile::remove(fileDateNames[i]);
}


void RollingFileAppender::computeRollOverTime()
{
  Q_ASSERT_X(!m_datePatternString.isEmpty(), "DailyRollingFileAppender::computeRollOverTime()", "No active date pattern");

  QDateTime now = QDateTime::currentDateTime();
  QDate nowDate = now.date();
  QTime nowTime = now.time();
  QDateTime start;

  switch (m_frequency)
  {
    case MinutelyRollover:
    {
      start = QDateTime(nowDate, nowTime);
      m_rollOverTime = start.addSecs(60);
    }
    break;
    case HourlyRollover:
    {
      start = QDateTime(nowDate, nowTime);
      m_rollOverTime = start.addSecs(60*60);
    }
    break;
    case HalfDailyRollover:
    {
      int hour = nowTime.hour();
      if (hour >=  12)
        hour = 12;
      else
        hour = 0;
      start = QDateTime(nowDate, nowTime);
      m_rollOverTime = start.addSecs(60*60*12);
    }
    break;
    case DailyRollover:
    {
      start = QDateTime(nowDate, nowTime);
      m_rollOverTime = start.addDays(1);
    }
    break;
    case WeeklyRollover:
    {
      // Qt numbers the week days 1..7. The week starts on Monday.
      // Change it to being numbered 0..6, starting with Sunday.
      int day = nowDate.dayOfWeek();
      if (day == Qt::Sunday)
        day = 0;
      start = QDateTime(nowDate, nowTime).addDays(-1 * day);
      m_rollOverTime = start.addDays(7);
    }
    break;
    case MonthlyRollover:
    {
      start = QDateTime(QDate(nowDate.year(), nowDate.month(), 1), nowTime);
      m_rollOverTime = start.addMonths(1);
    }
    break;
    default:
      Q_ASSERT_X(false, "DailyRollingFileAppender::computeInterval()", "Invalid datePattern constant");
      m_rollOverTime = QDateTime::fromTime_t(0);
  }

  m_rollOverSuffix = start.toString(m_datePatternString);
  Q_ASSERT_X(now.toString(m_datePatternString) == m_rollOverSuffix,
      "DailyRollingFileAppender::computeRollOverTime()", "File name changes within interval");
  Q_ASSERT_X(m_rollOverSuffix != m_rollOverTime.toString(m_datePatternString),
      "DailyRollingFileAppender::computeRollOverTime()", "File name does not change with rollover");
}


void RollingFileAppender::rollOver()
{
  Q_ASSERT_X(!m_datePatternString.isEmpty(), "DailyRollingFileAppender::rollOver()", "No active date pattern");

  QString rollOverSuffix = m_rollOverSuffix;
  computeRollOverTime();
  if (rollOverSuffix == m_rollOverSuffix)
    return;

  closeFile();

  QString targetFileName = fileName() + rollOverSuffix;
  QFile f(targetFileName);
  if (f.exists() && !f.remove())
    return;
  f.setFileName(fileName());
  if (!f.rename(targetFileName))
    return;

  openFile();
  removeOldFiles();
}


void RollingFileAppender::setLogFilesLimit(int limit)
{
  QMutexLocker locker(&m_rollingMutex);
  m_logFilesLimit = limit;
}


int RollingFileAppender::logFilesLimit() const
{
  QMutexLocker locker(&m_rollingMutex);
  return m_logFilesLimit;
}

DCORE_END_NAMESPACE
