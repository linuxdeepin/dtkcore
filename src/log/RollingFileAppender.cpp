// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

#include "RollingFileAppender.h"

DCORE_BEGIN_NAMESPACE

/*!
@~english
  @class Dtk::Core::RollingFileAppender
  @ingroup dlog
  @brief The RollingFileAppender class extends FileAppender so that the underlying file is rolled over at a user chosen frequency.

  The class is based on Log4Qt.DailyRollingFileAppender class (http://log4qt.sourceforge.net/)
  and has the same date pattern format.

  For example, if the fileName is set to /foo/bar and the DatePattern set to the daily rollover ('.'yyyy-MM-dd'.log'), on 2014-02-16 at midnight,
  the logging file /foo/bar.log will be copied to /foo/bar.2014-02-16.log and logging for 2014-02-17 will continue in /foo/bar
  until it rolls over the next day.

  The logFilesLimit parameter is used to automatically delete the oldest log files in the directory during rollover
  (so no more than logFilesLimit recent log files exist in the directory at any moment).
 */

RollingFileAppender::RollingFileAppender(const QString &fileName)
    : FileAppender(fileName),
      m_logFilesLimit(0),
      m_logSizeLimit(1024 * 1024 * 20)
{

}

void RollingFileAppender::append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                                 const char *func, const QString &category, const QString &msg)
{

    if (!m_rollOverTime.isNull() && QDateTime::currentDateTime() > m_rollOverTime)
        rollOver();

    if (size()> m_logSizeLimit)
        rollOver();

    FileAppender::append(time, level, file, line , func, category, msg);
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

void RollingFileAppender::setDatePattern(const QString &datePattern)
{
    setDatePatternString(datePattern);
    computeFrequency();

    computeRollOverTime();
}

void RollingFileAppender::setDatePatternString(const QString &datePatternString)
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
        // Weekly and Monthly 'start' time is changed, very different to 'now' time
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
        m_rollOverTime = QDateTime::fromSecsSinceEpoch(0);
    }

    m_rollOverSuffix = start.toString(m_datePatternString);
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

void RollingFileAppender::setLogSizeLimit(int limit)
{
    QMutexLocker locker(&m_rollingMutex);
    m_logSizeLimit = limit;
}

qint64 RollingFileAppender::logSizeLimit() const
{
    QMutexLocker locker(&m_rollingMutex);
    return m_logSizeLimit;
}

DCORE_END_NAMESPACE
