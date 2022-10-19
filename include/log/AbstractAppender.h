// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ABSTRACTAPPENDER_H
#define ABSTRACTAPPENDER_H

#include "dtkcore_global.h"
#include <Logger.h>

#include <QMutex>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT AbstractAppender
{
public:
    AbstractAppender();
    virtual ~AbstractAppender();

    Logger::LogLevel detailsLevel() const;
    void setDetailsLevel(Logger::LogLevel level);
    void setDetailsLevel(const QString &level);

    void write(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
               const char *func, const QString &category, const QString &msg);

protected:
    virtual void append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                        const char *func, const QString &category, const QString &msg) = 0;

private:
    QMutex m_writeMutex;

    Logger::LogLevel m_detailsLevel;
    mutable QMutex m_detailsLevelMutex;
};

DCORE_END_NAMESPACE
#endif // ABSTRACTAPPENDER_H
