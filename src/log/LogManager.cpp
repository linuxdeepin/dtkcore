// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LogManager.h"
#include "dsgapplication.h"
#include "dconfig.h"

#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>
#include <JournalAppender.h>

DCORE_BEGIN_NAMESPACE

#define RULES_KEY ("rules")
#define DEFAULT_FMT "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}\n"

/*!
@~english
  \class Dtk::Core::DLogManager
  \inmodule dtkcore

  \brief DLogManager is the deepin user application log manager.
 */

DLogManager::DLogManager()
    : m_format(DEFAULT_FMT)
    , m_dsgConfig(nullptr)
    , m_fallbackConfig(nullptr)
{
    initLoggingRules();
}

DConfig *DLogManager::createDConfig(const QString &appId)
{
    if (appId.isEmpty())
        return nullptr;

    DConfig *config = DConfig::create(appId, "org.deepin.dtk.loggingrules");
    if (!config->isValid()) {
        qWarning() << "Logging rules config is invalid, please check `appId` [" << appId << "]arg is correct";
        delete config;
        config = nullptr;
        return nullptr;
    }

    QObject::connect(config, &DConfig::valueChanged, config, [this](const QString &key) {
        if (key != RULES_KEY)
            return;

        updateLoggingRules();
    });

    return config;
}

void DLogManager::initLoggingRules()
{
    if (qEnvironmentVariableIsSet("DTK_DISABLED_LOGGING_RULES"))
        return;

    // 1. 未指定 fallbackId 时，以 dsgAppId 为准
    QString dsgAppId = DSGApplication::id();
    m_dsgConfig = createDConfig(dsgAppId);

    QString fallbackId = qgetenv("DTK_LOGGING_FALLBACK_APPID");
    // 2. fallbackId 和 dsgAppId 非空且不等时，都创建和监听变化
    if (!fallbackId.isEmpty() && fallbackId != dsgAppId)
        m_fallbackConfig = createDConfig(fallbackId);

    updateLoggingRules();
}

void DLogManager::updateLoggingRules()
{
    QVariant var;
    // 3. 优先看 fallback 是否存在
    if (m_fallbackConfig) {
        var = m_fallbackConfig->value(RULES_KEY);
    } else if (m_dsgConfig) {
        var = m_dsgConfig->value(RULES_KEY);
    }

    if (var.isValid())
        QLoggingCategory::setFilterRules(var.toString().replace(";", "\n"));
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

void DLogManager::registerLoggingRulesWatcher(const QString &appId)
{
    Q_UNUSED(appId)
    qWarning() << "This interface has been deprecated, but the functionality is enabled by default"
                  ", which can be disabled by environment variables: DTK_DISABLED_LOGGING_RULES";
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
    if (m_dsgConfig) {
        delete m_dsgConfig;
        m_dsgConfig = nullptr;
    }

    if (m_fallbackConfig) {
        delete m_fallbackConfig;
        m_fallbackConfig = nullptr;
    }
}

DCORE_END_NAMESPACE
