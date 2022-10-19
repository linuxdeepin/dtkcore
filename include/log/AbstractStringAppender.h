// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ABSTRACTSTRINGAPPENDER_H
#define ABSTRACTSTRINGAPPENDER_H

#include "AbstractAppender.h"

#include <QReadWriteLock>
#include <QDateTime>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT AbstractStringAppender : public AbstractAppender
{
public:
    AbstractStringAppender();
    virtual QString format() const;
    void setFormat(const QString &format);

    static QString stripFunctionName(const char *name);
protected:
    QString formattedString(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                            const char *func, const QString &category, const QString &msg) const;

private:
    static QByteArray qCleanupFuncinfo(const char*);

    QString m_format;
    mutable QReadWriteLock m_formatLock;
};

DCORE_END_NAMESPACE
#endif // ABSTRACTSTRINGAPPENDER_H
