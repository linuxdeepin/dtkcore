// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FILEAPPENDER_H
#define FILEAPPENDER_H

// Logger
#include "dtkcore_global.h"
#include <AbstractStringAppender.h>

// Qt
#include <QFile>
#include <QTextStream>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT FileAppender : public AbstractStringAppender
{
public:
    FileAppender(const QString &fileName = QString());
    ~FileAppender();

    QString fileName() const;
    void setFileName(const QString &s);

    qint64 size() const;

protected:
    virtual void append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                        const char *func, const QString &category, const QString &msg);
    bool openFile();
    void closeFile();

private:
    QFile m_logFile;
    QTextStream m_logStream;
    mutable QMutex m_logFileMutex;
};

DCORE_END_NAMESPACE

#endif // FILEAPPENDER_H
