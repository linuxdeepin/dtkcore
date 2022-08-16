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

#ifndef ROLLINGFILEAPPENDER_H
#define ROLLINGFILEAPPENDER_H

#include <QDateTime>

#include <FileAppender.h>

DCORE_BEGIN_NAMESPACE

class CUTELOGGERSHARED_EXPORT RollingFileAppender : public FileAppender
{
  public:
    /*!
  The enum DatePattern defines constants for date patterns.
  \sa setDatePattern(DatePattern)
     */
    enum DatePattern
    {
      /*! The minutely date pattern string is "'.'yyyy-MM-dd-hh-mm". */
      MinutelyRollover = 0,
      /*! The hourly date pattern string is "'.'yyyy-MM-dd-hh". */
      HourlyRollover,
      /*! The half-daily date pattern string is "'.'yyyy-MM-dd-a". */
      HalfDailyRollover,
      /*! The daily date pattern string is "'.'yyyy-MM-dd". */
      DailyRollover,
      /*! The weekly date pattern string is "'.'yyyy-ww". */
      WeeklyRollover,
      /*! The monthly date pattern string is "'.'yyyy-MM". */
      MonthlyRollover
    };

    RollingFileAppender(const QString& fileName = QString());

    DatePattern datePattern() const;
    void setDatePattern(DatePattern datePattern);
    void setDatePattern(const QString& datePattern);

    QString datePatternString() const;

    void setLogFilesLimit(int limit);
    int logFilesLimit() const;

    void setLogSizeLimit(int qint64);
    qint64 logSizeLimit() const;

  protected:
    virtual void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                        const char* function, const QString& category, const QString& message);

  private:
    void rollOver();
    void computeRollOverTime();
    void computeFrequency();
    void removeOldFiles();
    void setDatePatternString(const QString& datePatternString);

    QString m_datePatternString;
    DatePattern m_frequency;

    QDateTime m_rollOverTime;
    QString m_rollOverSuffix;
    int m_logFilesLimit;
    qint64 m_logSizeLimit;
    mutable QMutex m_rollingMutex;
};

DCORE_END_NAMESPACE

#endif // ROLLINGFILEAPPENDER_H
