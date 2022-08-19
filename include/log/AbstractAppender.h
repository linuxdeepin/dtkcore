// SPDX-FileCopyrightText: 2010 Boris Moiseev (cyberbobs at gmail dot com)
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ABSTRACTAPPENDER_H
#define ABSTRACTAPPENDER_H

// Local
#include "CuteLogger_global.h"
#include <Logger.h>

// Qt
#include <QMutex>

DCORE_BEGIN_NAMESPACE

class CUTELOGGERSHARED_EXPORT AbstractAppender
{
public:
    AbstractAppender();
    virtual ~AbstractAppender();

    Logger::LogLevel detailsLevel() const;
    void setDetailsLevel(Logger::LogLevel level);
    void setDetailsLevel(const QString &level);

    void write(const QDateTime &timeStamp, Logger::LogLevel logLevel, const char *file, int line, const char *function,
               const QString &category, const QString &message);

protected:
    virtual void append(const QDateTime &timeStamp, Logger::LogLevel logLevel, const char *file, int line,
                        const char *function, const QString &category, const QString &message) = 0;

private:
    QMutex m_writeMutex;

    Logger::LogLevel m_detailsLevel;
    mutable QMutex m_detailsLevelMutex;
};

DCORE_END_NAMESPACE

#endif // ABSTRACTAPPENDER_H
