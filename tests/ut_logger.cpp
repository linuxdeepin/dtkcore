// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QFile>
#include <QLocale>
#include "log/Logger.h"
#include "log/FileAppender.h"
#include "log/ConsoleAppender.h"
#include "log/RollingFileAppender.h"

DCORE_USE_NAMESPACE

class ut_Logger: public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    Logger *m_logger = nullptr;
};

void ut_Logger::SetUp()
{
    m_logger = new Logger;
    QFile file("/tmp/log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.close();
    QFile rollFile("/tmp/rollLog");
    if (!rollFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    rollFile.close();
}

void ut_Logger::TearDown()
{
    if (m_logger) {
        delete m_logger;
        m_logger = nullptr;
    }
    QFile file("/tmp/log");
    if (file.exists())
        file.remove();
    QFile rollFile("/tmp/rollLog");
    if (rollFile.exists())
        rollFile.remove();
}

TEST_F(ut_Logger, testLevelToString)
{
    QString trace = Logger::levelToString(Logger::Trace);
    ASSERT_EQ(trace, "Trace");
    QString debug = Logger::levelToString(Logger::Debug);
    ASSERT_EQ(debug, "Debug");
    QString info = Logger::levelToString(Logger::Info);
    ASSERT_EQ(info, "Info");
    QString warning = Logger::levelToString(Logger::Warning);
    ASSERT_EQ(warning, "Warning");
    QString error = Logger::levelToString(Logger::Error);
    ASSERT_EQ(error, "Error");
    QString fatal = Logger::levelToString(Logger::Fatal);
    ASSERT_EQ(fatal, "Fatal");
}

TEST_F(ut_Logger, testLevelFromString)
{
    Logger::LogLevel trace = Logger::levelFromString("Trace");
    ASSERT_EQ(trace, Logger::Trace);
    Logger::LogLevel debug = Logger::levelFromString("Debug");
    ASSERT_EQ(debug, Logger::Debug);
    Logger::LogLevel info = Logger::levelFromString("Info");
    ASSERT_EQ(info, Logger::Info);
    Logger::LogLevel warning = Logger::levelFromString("Warning");
    ASSERT_EQ(warning, Logger::Warning);
    Logger::LogLevel error = Logger::levelFromString("Error");
    ASSERT_EQ(error, Logger::Error);
    Logger::LogLevel fatal = Logger::levelFromString("Fatal");
    ASSERT_EQ(fatal, Logger::Fatal);
}

TEST_F(ut_Logger, testGlobalInstance)
{
    ASSERT_TRUE(Logger::globalInstance());
}

TEST_F(ut_Logger, testRegisterAppender)
{
    Logger* gLogger = Logger::globalInstance();

    FileAppender *fileAppener = new FileAppender("/tmp/log");
    if (fileAppener->detailsLevel() > Logger::Trace)
        fileAppener->setDetailsLevel(Logger::Trace);
    gLogger->registerAppender(fileAppener);
    ASSERT_TRUE(fileAppener->size() == 0);
    dTrace("testRegisterAppender");
    ASSERT_TRUE(fileAppener->size() != 0);

    ConsoleAppender *consoleAppener = new ConsoleAppender();
    if (consoleAppener->detailsLevel() > Logger::Trace)
        consoleAppener->setDetailsLevel(Logger::Trace);
    gLogger->registerAppender(consoleAppener);
    consoleAppener->ignoreEnvironmentPattern(false);
    QString format = consoleAppener->format();
    consoleAppener->setFormat("[%{file}: %{line} %{type:-7}] <%{function}> %{message}\n");
    dTrace("testRegisterAppender");
}


TEST_F(ut_Logger, testRegisterCategoryAppender)
{
    Logger* gLogger = Logger::globalInstance();
    FileAppender *fileAppener = new FileAppender("/tmp/log");
    if (fileAppener->detailsLevel() > Logger::Trace)
        fileAppener->setDetailsLevel(Logger::Trace);
    gLogger->registerCategoryAppender("test", fileAppener);
    ASSERT_TRUE(fileAppener->size() == 0);
    dCDebug("test") << "testRegisterAppender";
    ASSERT_TRUE(fileAppener->size() != 0);
}

TEST_F(ut_Logger, testLogToGlobalInstance)
{
    Logger* gLogger = Logger::globalInstance();
    FileAppender *fileAppener = new FileAppender("/tmp/log");
    if (fileAppener->detailsLevel() > Logger::Trace)
        fileAppener->setDetailsLevel(Logger::Trace);
    gLogger->registerAppender(fileAppener);
    m_logger->logToGlobalInstance("test", true);

    ASSERT_TRUE(fileAppener->size() == 0);
    dTrace("testRegisterAppender");
    ASSERT_TRUE(fileAppener->size() != 0);
}

TEST_F(ut_Logger, testSetDefaultCategory)
{
    m_logger->setDefaultCategory("test");
    ASSERT_EQ(m_logger->defaultCategory(), "test");
}

TEST_F(ut_Logger, testDefaultCategory)
{
    ASSERT_EQ(m_logger->defaultCategory(), "");
}

TEST_F(ut_Logger, testRollingFileAppender)
{
    Logger* gLogger = Logger::globalInstance();

    RollingFileAppender *rfa = new RollingFileAppender("/tmp/rollLog");

    if (rfa->detailsLevel() > Logger::Trace)
        rfa->setDetailsLevel(Logger::Trace);

    gLogger->registerAppender(rfa);

    rfa->setLogFilesLimit(2);
    ASSERT_TRUE(rfa->logFilesLimit() == 2);


    rfa->setDatePattern("'.'yyyy-MM-dd-hh-mm");
    ASSERT_TRUE(rfa->datePattern() == RollingFileAppender::MinutelyRollover);
    ASSERT_TRUE(rfa->datePatternString() == "'.'yyyy-MM-dd-hh-mm");

    rfa->setDatePattern("'.'yyyy-MM-dd-hh");
    ASSERT_TRUE(rfa->datePattern() == RollingFileAppender::HourlyRollover);
    ASSERT_TRUE(rfa->datePatternString() == "'.'yyyy-MM-dd-hh");

    // pattern string set to default
    rfa->setDatePattern(RollingFileAppender::HalfDailyRollover);
    ASSERT_TRUE(rfa->datePattern() == RollingFileAppender::HalfDailyRollover);
    ASSERT_TRUE(rfa->datePatternString() == "'.'yyyy-MM-dd-hh-mm-ss-zzz");

    rfa->setDatePattern("'.'yyyy-MM-dd");
    ASSERT_TRUE(rfa->datePattern() == RollingFileAppender::DailyRollover);
    ASSERT_TRUE(rfa->datePatternString() == "'.'yyyy-MM-dd");

    // pattern string set to default
    rfa->setDatePattern(RollingFileAppender::WeeklyRollover);
    ASSERT_TRUE(rfa->datePattern() == RollingFileAppender::WeeklyRollover);
    ASSERT_TRUE(rfa->datePatternString() == "'.'yyyy-MM-dd-hh-mm-ss-zzz");

    rfa->setDatePattern("'.'yyyy-MM");
    ASSERT_TRUE(rfa->datePattern() == RollingFileAppender::MonthlyRollover);
    ASSERT_TRUE(rfa->datePatternString() == "'.'yyyy-MM");

    // dTrace("testRegisterAppender");
}
