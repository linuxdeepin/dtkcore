// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ROLLINGFILEAPPENDER_H
#define ROLLINGFILEAPPENDER_H

#include <QDateTime>

#include <FileAppender.h>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT RollingFileAppender : public FileAppender
{
  public:
    /*!
    @~english
    The enum DatePattern defines constants for date patterns.
    \sa setDatePattern(DatePattern)
     */
    enum DatePattern
    {

      MinutelyRollover = 0,/*!<@~english The minutely date pattern string is "'.'yyyy-MM-dd-hh-mm". */
      HourlyRollover,/*!<@~english The hourly date pattern string is "yyyy-MM-dd-hh". */
      HalfDailyRollover,/*!<@~english The half-daily date pattern string is "'.'yyyy-MM-dd-a". */
      DailyRollover,/*!<@~english The daily date pattern string is "'.'yyyy-MM-dd". */
      WeeklyRollover,/*!<@~english The weekly date pattern string is "'.'yyyy-ww". */
      MonthlyRollover/*!<@~english The monthly date pattern string is "'.'yyyy-MM". */
    };

    RollingFileAppender(const QString &fileName = QString());

    DatePattern datePattern() const;
    void setDatePattern(DatePattern datePattern);
    void setDatePattern(const QString &datePattern);

    QString datePatternString() const;

    void setLogFilesLimit(int limit);
    int logFilesLimit() const;

    void setLogSizeLimit(int limit);
    qint64 logSizeLimit() const;

  protected:
    virtual void append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                        const char *func, const QString &category, const QString &msg);

  private:
    void rollOver();
    void computeRollOverTime();
    void computeFrequency();
    void removeOldFiles();
    void setDatePatternString(const QString &datePatternString);

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
