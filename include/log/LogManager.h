// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QtCore>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class ConsoleAppender;
class RollingFileAppender;
class JournalAppender;

class LIBDTKCORESHARED_EXPORT DLogManager
{
public:
    static void registerConsoleAppender();
    static void registerFileAppender();
    static void registerJournalAppender();

    static QString getlogFilePath();

    /*!
     * \brief setlogFilePath will change log file path of registerFileAppender
     * \a logFilePath is the full path of file appender log
     */
    static void setlogFilePath(const QString &logFilePath);

    static void setLogFormat(const QString &format);

private:
//TODO: move it to private class (DTK6)
    QString m_format;
    QString m_logPath;
    ConsoleAppender* m_consoleAppender;
    RollingFileAppender* m_rollingFileAppender;
    JournalAppender* m_journalAppender;

    void initConsoleAppender();
    void initRollingFileAppender();
    void initJournalAppender();
    QString joinPath(const QString &path, const QString &fileName);

    inline static DLogManager* instance(){
        static DLogManager instance;
        return &instance;
    }
    explicit DLogManager();
    ~DLogManager();
    DLogManager(const DLogManager &);
    DLogManager & operator = (const DLogManager &);
};

DCORE_END_NAMESPACE

#endif // LOGMANAGER_H
