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

#include "LogManager.h"
#include <DConfig>
#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>

DCORE_BEGIN_NAMESPACE

/*!
  \class Dtk::Core::DLogManager
  \inmodule dtkcore
  
  \brief DLogManager is the deepin user application log manager.
 */

DLogManager::DLogManager()
{
    // 不加这两个宏中的任一个则无法打印文件名、行号、函数名，那就只打印时间、Log类型、内容
#if !defined(QT_DEBUG) && !defined(QT_MESSAGELOGCONTEXT)
    m_format = "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] %{message}\n";
#else
    m_format = CUTELOGGER_DEFAULT_LOG_FORMAT;
#endif
}

void DLogManager::initConsoleAppender(){
    QString logFormat = m_format;
    DConfig config("org.deepin.dtkcore");
    if (config.isValid()) {
        QString fmt = config.value("logFormat").toString();
        if (!fmt.isEmpty()) {
            fmt.replace("\\n", "\n");
            logFormat = fmt;
        }
    }

    m_consoleAppender = new ConsoleAppender;
    m_consoleAppender->setFormat(logFormat);
    logger->registerAppender(m_consoleAppender);
}

void DLogManager::initRollingFileAppender(){
    int rollingNum = 5;
    Logger::LogLevel level = Logger::LogLevel::Trace;
    RollingFileAppender::DatePattern pattern = RollingFileAppender::DailyRollover;
    QString logFilePath;
    QString logFormat = m_format;

    DConfig config("org.deepin.dtkcore");

    if (!config.isValid()) {
        qWarning() << QString("DConfig is invalide, name:[%1], subpath[%2].").arg(config.name(), config.subpath());
    } else {
        bool success;

        // get rollingNumber, default is 5
        int num = config.value("rollingNumber").toInt(&success);
        if (success && num > 0)
            rollingNum = num;

        // get datePattern, default is DailyRollover
        num = config.value("datePattern").toInt(&success);
        if (success && num >= static_cast<RollingFileAppender::DatePattern>(RollingFileAppender::MinutelyRollover)
                && num <= static_cast<RollingFileAppender::DatePattern>(RollingFileAppender::MonthlyRollover)) {
            pattern = static_cast<RollingFileAppender::DatePattern>(num);
        }
        // get logLevel, default is Trace
        num = config.value("logLevel").toInt(&success);
        if (success && num >= static_cast<Logger::LogLevel>(Logger::LogLevel::Trace)
                && num <= static_cast<Logger::LogLevel>(Logger::LogLevel::Fatal)) {
            level = static_cast<Logger::LogLevel>(num);
        }
        // get logPath, default is Trace
        if (DLogManager::instance()->m_logPath.isEmpty()) {
            // 从 dconfig 获取配置的优先级更高
            QString path = config.value("logPath").toString();
            if (!path.isEmpty() && QFileInfo(path).exists()) {
                m_logPath = DLogManager::instance()->joinPath(path, QString("%1.log").arg(qApp->applicationName()));
                logFilePath = QDir::toNativeSeparators(m_logPath);
            }
        }
        QString fmt = config.value("logFormat").toString();
        if (!fmt.isEmpty()) {
            fmt.replace("\\n", "\n");
            logFormat = fmt;
        }
    }
    if (logFilePath.isEmpty()) {
        logFilePath = getlogFilePath();
    }

    m_rollingFileAppender = new RollingFileAppender(logFilePath);
    m_rollingFileAppender->setFormat(logFormat);
    m_rollingFileAppender->setLogFilesLimit(rollingNum);
    m_rollingFileAppender->setDatePattern(pattern);
    m_rollingFileAppender->setDetailsLevel(level);
    logger->registerAppender(m_rollingFileAppender);
}

/*!
  \brief Registers the appender to write the log records to the Console.

  \sa registerFileAppender
 */
void DLogManager::registerConsoleAppender(){
    DLogManager::instance()->initConsoleAppender();
}

/*!
  \brief Registers the appender to write the log records to the file.

  \sa getlogFilePath
  \sa registerConsoleAppender
 */
void DLogManager::registerFileAppender() {
    DLogManager::instance()->initRollingFileAppender();
}

/*!
  \brief Return the path file log storage.

  \brief DLogManager::getlogFilePath 获取日志文件路径
  \brief 默认日志路径是 ~/.cache/organizationName/applicationName.log
  \brief 如果获取 HOME 环境变量失败将不写日志
  \sa registerFileAppender
 */
QString DLogManager::getlogFilePath()
{
    // 不再构造时去设置默认logpath(且mkdir), 而在getlogPath时再去判断是否设置默认值
    // 修复设置了日志路径还是会在默认的位置创建目录的问题
    if (DLogManager::instance()->m_logPath.isEmpty()) {
        if (QDir::homePath() == QDir::rootPath()) {
            qWarning() << "unable to locate the cache directory."
                       << "logfile path is empty, the log will not be written.\r\n"
                       << (qgetenv("HOME").isEmpty() ? "the HOME environment variable not set" : "");
            return QString();
        }

        QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
        if (!QDir(cachePath).exists()) {
            QDir(cachePath).mkpath(cachePath);
        }
        DLogManager::instance()->m_logPath = DLogManager::instance()->joinPath(cachePath, QString("%1.log").arg(qApp->applicationName()));
    }

    return QDir::toNativeSeparators(DLogManager::instance()->m_logPath);
}

/*!
  \brief DLogManager::setlogFilePath 设置日志文件路径
  \a logFilePath 日志文件路径
  \brief 如果设置的文件路进不是文件路径将什么都不做，输出一条警告
 */
void DLogManager::setlogFilePath(const QString &logFilePath)
{
    QFileInfo info(logFilePath);
    if (info.exists() && !info.isFile())
        qWarning() << "invalid file path:" << logFilePath << " is not a file";
    else
        DLogManager::instance()->m_logPath = logFilePath;
}

void DLogManager::setLogFormat(const QString &format)
{
    //m_format = "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}\n";
    DLogManager::instance()->m_format = format;
}

QString DLogManager::joinPath(const QString &path, const QString &fileName){
    QString separator(QDir::separator());
    return QString("%1%2%3").arg(path, separator, fileName);
}

DLogManager::~DLogManager()
{

}

DCORE_END_NAMESPACE
