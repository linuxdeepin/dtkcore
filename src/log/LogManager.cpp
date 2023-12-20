// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LogManager.h"
#include "dconfig.h"

#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>
#include <JournalAppender.h>

DCORE_BEGIN_NAMESPACE

#define RULES_KEY ("rules")

/*!
@~english
  \class Dtk::Core::DLogManager
  \inmodule dtkcore

  \brief DLogManager is the deepin user application log manager.
 */

DLogManager::DLogManager()
    : m_loggingRulesConfig(nullptr)
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


void DLogManager::initJournalAppender()
{
#if (defined BUILD_WITH_SYSTEMD && defined Q_OS_LINUX)
    m_journalAppender = new JournalAppender();
    logger->registerAppender(m_journalAppender);
#endif
}

/*!
@~english
  \brief Registers the appender to write the log records to the Console.

  \sa registerFileAppender
 */
void DLogManager::registerConsoleAppender(){
    DLogManager::instance()->initConsoleAppender();
}

/*!
@~english
  \brief Registers the appender to write the log records to the file.

  \sa getlogFilePath
  \sa registerConsoleAppender
 */
void DLogManager::registerFileAppender() {
    DLogManager::instance()->initRollingFileAppender();
}

void DLogManager::registerJournalAppender()
{
    DLogManager::instance()->initJournalAppender();
}

void DLogManager::initLoggingRules(const QString &appId)
{
    if (appId.isEmpty()) {
        qWarning() << "App id is empty, logging rules won't take effect";
        return;
    }

    // QT_LOGGING_RULES环境变量设置日志的优先级最高
    // QLoggingRegistry 初始化时会获取 QT_LOGGING_RULES 的值并保存，后续重置了环境变量 QLoggingRegistry 不会进行同步
    // 需要在 QLoggingRegistry 初始化之前重置 QT_LOGGING_RULES 的值
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    qunsetenv("QT_LOGGING_RULES");

    if (!logRules.isEmpty()) {
        QLoggingCategory::setFilterRules(logRules.replace(";", "\n"));
    }

    m_loggingRulesConfig = DConfig::create(appId, "org.deepin.dtk.loggingrules");
    if (!m_loggingRulesConfig) {
        qWarning() << "Create logging rules dconfig object failed, logging rules won't take effect";
        return;
    }

    if (!m_loggingRulesConfig->isValid()) {
        qWarning() << "Logging rules config is invalid, please check `appId` arg is correct";
        delete m_loggingRulesConfig;
        m_loggingRulesConfig = nullptr;
        return;
    }

    auto updateLoggingRules = [this](const QString & key) {
        if (key != RULES_KEY)
            return;

        const QVariant &var = m_loggingRulesConfig->value(RULES_KEY);
        if (var.isValid() && !var.toString().isEmpty()) {
            QLoggingCategory::setFilterRules(var.toString().replace(";", "\n"));
        }
    };

    updateLoggingRules(RULES_KEY);
    QObject::connect(m_loggingRulesConfig, &DConfig::valueChanged, m_loggingRulesConfig, updateLoggingRules);
}

void DLogManager::registerLoggingRulesWatcher(const QString &appId)
{
    DLogManager::instance()->initLoggingRules(appId);
}

/*!
@~english
  \brief Return the path file log storage.

  \brief DLogManager::getlogFilePath Get the log file path
  \brief The default log path is ~/.cache/<OrganizationName>/<ApplicationName>.log
  \brief If the environment variable $HOME cannot be acquired, DLogManager will not log anything
  \sa registerFileAppender
 */
QString DLogManager::getlogFilePath()
{
    //No longer set the default log path (and mkdir) when constructing now, instead set the default value if it's empty when getlogFilePath is called.
    //Fix the problem that the log file is still created in the default path when the log path is set manually.
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
@~english
  \brief DLogManager::setlogFilePath Set the log file path
  \a logFilePath Log file path
  \brief If the file path set is not the file path, nothing will do, and an output warning
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
    if (m_loggingRulesConfig) {
        delete m_loggingRulesConfig;
        m_loggingRulesConfig = nullptr;
    }
}

DCORE_END_NAMESPACE
