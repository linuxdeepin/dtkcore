// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QScopedPointer>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DLogManagerPrivate;
class LIBDTKCORESHARED_EXPORT DLogManager
{
    Q_DISABLE_COPY(DLogManager)
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

    /*!
     * \brief 监听 org.deepin.dtk.log 的变化动态调整应用的日志输出规则
     * 此方法应该在创建 QApplication 之前调用，否则 QT_LOGGING_RULES 环境变量会覆盖 dconfig 的的值
     * \a logFilePath 指定 dconfig 的 appId
     */
    static void registerLoggingRulesWatcher(const QString &appId);

private:
    void initConsoleAppender();
    void initRollingFileAppender();
    void initJournalAppender();
    void initLoggingRules(const QString &appId);
    QString joinPath(const QString &path, const QString &fileName);

    inline static DLogManager* instance(){
        static DLogManager instance;
        return &instance;
    }
    explicit DLogManager();
    ~DLogManager();

    QScopedPointer<DLogManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DLogManager)
};

DCORE_END_NAMESPACE

#endif // LOGMANAGER_H
