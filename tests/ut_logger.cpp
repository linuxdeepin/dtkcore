/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Wang Fei <wangfeia@uniontech.com>
 *
 * Maintainer: Wang Fei <wangfeia@uniontech.com>
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

    RollingFileAppender *rollingFileAppender = new RollingFileAppender("/tmp/rollLog");
    if (rollingFileAppender->detailsLevel() > Logger::Trace)
        rollingFileAppender->setDetailsLevel(Logger::Trace);
    gLogger->registerAppender(rollingFileAppender);
    rollingFileAppender->setDatePattern("'.'yyyy-MM-dd-hh-mm");
    ASSERT_TRUE(rollingFileAppender->datePatternString() == "'.'yyyy-MM-dd-hh-mm");
    rollingFileAppender->setLogFilesLimit(2);
    ASSERT_TRUE(rollingFileAppender->logFilesLimit() == 2);
    rollingFileAppender->setDatePattern(RollingFileAppender::MinutelyRollover);
    ASSERT_TRUE(rollingFileAppender->datePattern() == RollingFileAppender::MinutelyRollover);
    dTrace("testRegisterAppender");
    rollingFileAppender->setDatePattern(RollingFileAppender::HourlyRollover);
    ASSERT_TRUE(rollingFileAppender->datePattern() == RollingFileAppender::HourlyRollover);
    dTrace("testRegisterAppender");
    rollingFileAppender->setDatePattern(RollingFileAppender::HalfDailyRollover);
    ASSERT_TRUE(rollingFileAppender->datePattern() == RollingFileAppender::HalfDailyRollover);
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
