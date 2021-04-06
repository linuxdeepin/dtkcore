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
#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>

DCORE_BEGIN_NAMESPACE

/**
 * \class DLogManager
 *
 * \brief DLogManager is the deepin user application log manager
 */

DLogManager::DLogManager()
{
    m_format = "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}\n";
}

void DLogManager::initConsoleAppender(){
    m_consoleAppender = new ConsoleAppender;
    m_consoleAppender->setFormat(m_format);
    logger->registerAppender(m_consoleAppender);
}

void DLogManager::initRollingFileAppender(){
    m_rollingFileAppender = new RollingFileAppender(getlogFilePath());
    m_rollingFileAppender->setFormat(m_format);
    m_rollingFileAppender->setLogFilesLimit(5);
    m_rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);
    logger->registerAppender(m_rollingFileAppender);
}

//! Registers the appender to write the log records to the Console
/**
 * \sa registerFileAppender
 */
void DLogManager::registerConsoleAppender(){
    DLogManager::instance()->initConsoleAppender();
}

//! Registers the appender to write the log records to the file
/**
 * \sa getlogFilePath
 * \sa registerConsoleAppender
 */
void DLogManager::registerFileAppender() {
    DLogManager::instance()->initRollingFileAppender();
}

//! Return the path file log storage
/**
 * \sa registerFileAppender
 */
QString DLogManager::getlogFilePath(){
    // 不再构造时去设置默认logpath(且mkdir), 而在getlogPath时再去判断是否设置默认值
    // 修复设置了日志路径还是会在默认的位置创建目录的问题
    if (DLogManager::instance()->m_logPath.isEmpty()) {
        QString cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).at(0);
        if (!QDir(cachePath).exists()) {
            QDir(cachePath).mkpath(cachePath);
        }
        DLogManager::instance()->m_logPath = DLogManager::instance()->joinPath(cachePath, QString("%1.log").arg(qApp->applicationName()));
    }

    return QDir::toNativeSeparators(DLogManager::instance()->m_logPath);
}


/*!
 * \~chinese \brief DLogManager::setlogFilePath 设置日志文件路径
 * \~chinese \param logFilePath 日志文件路径
 */
void DLogManager::setlogFilePath(const QString &logFilePath)
{
    if (QDir(logFilePath).exists())
        DLogManager::instance()->m_logPath = DLogManager::instance()->joinPath(logFilePath, QString("%1.log").arg(qApp->applicationName()));
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
