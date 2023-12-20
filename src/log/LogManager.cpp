// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QtCore>
#include "LogManager.h"
#include "dconfig.h"

#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>
#include <JournalAppender.h>
#include "dstandardpaths.h"

#include "spdlog/spdlog.h"

DCORE_BEGIN_NAMESPACE

#define RULES_KEY ("rules")

// Courtesy qstandardpaths_unix.cpp
static void appendOrganizationAndApp(QString &path)
{
#ifndef QT_BOOTSTRAPPED
    const QString org = QCoreApplication::organizationName();
    if (!org.isEmpty())
        path += QLatin1Char('/') + org;
    const QString appName = QCoreApplication::applicationName();
    if (!appName.isEmpty())
        path += QLatin1Char('/') + appName;
#else
    Q_UNUSED(path);
#endif
}

#define DEFAULT_FMT "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}"

class DLogManagerPrivate {
public:
    explicit DLogManagerPrivate(DLogManager *q)
        : m_format(DEFAULT_FMT)
        , q_ptr(q)
    {
    }
    ~DLogManagerPrivate() {
        if (m_loggingRulesConfig) {
            delete m_loggingRulesConfig;
            m_loggingRulesConfig = nullptr;
        }
    }

    QString m_format;
    QString m_logPath;
    ConsoleAppender* m_consoleAppender = nullptr;
    RollingFileAppender* m_rollingFileAppender = nullptr;
    JournalAppender* m_journalAppender = nullptr;
    DConfig* m_loggingRulesConfig = nullptr;

    DLogManager *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(DLogManager)

};
/*!
@~english
  \class Dtk::Core::DLogManager
  \inmodule dtkcore

  \brief DLogManager is the deepin user application log manager.
 */

DLogManager::DLogManager()
    :d_ptr(new DLogManagerPrivate(this))
{
    spdlog::set_automatic_registration(true);
    spdlog::set_pattern("%v");
}

void DLogManager::initConsoleAppender(){
    Q_D(DLogManager);
    d->m_consoleAppender = new ConsoleAppender;
    d->m_consoleAppender->setFormat(d->m_format);
    dlogger->registerAppender(d->m_consoleAppender);
}

void DLogManager::initRollingFileAppender(){
    Q_D(DLogManager);
    d->m_rollingFileAppender = new RollingFileAppender(getlogFilePath());
    d->m_rollingFileAppender->setFormat(d->m_format);
    d->m_rollingFileAppender->setLogFilesLimit(5);
    d->m_rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);
    dlogger->registerAppender(d->m_rollingFileAppender);
}

void DLogManager::initJournalAppender()
{
#if (defined BUILD_WITH_SYSTEMD && defined Q_OS_LINUX)
    Q_D(DLogManager);
    d->m_journalAppender = new JournalAppender();
    dlogger->registerAppender(d->m_journalAppender);
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

    Q_D(DLogManager);
    d->m_loggingRulesConfig = DConfig::create(appId, "org.deepin.dtk.loggingrules");
    if (!d->m_loggingRulesConfig) {
        qWarning() << "Create logging rules dconfig object failed, logging rules won't take effect";
        return;
    }

    if (!d->m_loggingRulesConfig->isValid()) {
        qWarning() << "Logging rules config is invalid, please check `appId` arg is correct";
        delete d->m_loggingRulesConfig;
        d->m_loggingRulesConfig = nullptr;
        return;
    }

    auto updateLoggingRules = [d](const QString & key) {
        if (key != RULES_KEY)
            return;

        const QVariant &var = d->m_loggingRulesConfig->value(RULES_KEY);
        if (var.isValid() && !var.toString().isEmpty()) {
            QLoggingCategory::setFilterRules(var.toString().replace(";", "\n"));
        }
    };

    updateLoggingRules(RULES_KEY);
    QObject::connect(d->m_loggingRulesConfig, &DConfig::valueChanged, d->m_loggingRulesConfig, updateLoggingRules);
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
    if (DLogManager::instance()->d_func()->m_logPath.isEmpty()) {
        if (DStandardPaths::homePath().isEmpty()) {
            qWarning() << "Unable to locate the cache directory, cannot acquire home directory, and the log will not be written to file..";
            return QString();
        }

        QString cachePath(DStandardPaths::path(DStandardPaths::XDG::CacheHome));
        appendOrganizationAndApp(cachePath);

        if (!QDir(cachePath).exists()) {
            QDir(cachePath).mkpath(cachePath);
        }
        instance()->d_func()->m_logPath = instance()->joinPath(cachePath, QString("%1.log").arg(qApp->applicationName()));
    }

    return QDir::toNativeSeparators(DLogManager::instance()->d_func()->m_logPath);
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
        DLogManager::instance()->d_func()->m_logPath = logFilePath;
}

void DLogManager::setLogFormat(const QString &format)
{
    //m_format = "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}\n";
    DLogManager::instance()->d_func()->m_format = format;
}

QString DLogManager::joinPath(const QString &path, const QString &fileName){
    QString separator(QDir::separator());
    return QString("%1%2%3").arg(path, separator, fileName);
}

DLogManager::~DLogManager()
{
    spdlog::shutdown();
}

DCORE_END_NAMESPACE
