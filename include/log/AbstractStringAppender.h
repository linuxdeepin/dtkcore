// SPDX-FileCopyrightText: 2010 Boris Moiseev (cyberbobs at gmail dot com)
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ABSTRACTSTRINGAPPENDER_H
#define ABSTRACTSTRINGAPPENDER_H

// Local
#include "CuteLogger_global.h"
#include <AbstractAppender.h>

// Qt
#include <QReadWriteLock>

DCORE_BEGIN_NAMESPACE

class CUTELOGGERSHARED_EXPORT AbstractStringAppender : public AbstractAppender
{
  public:
    AbstractStringAppender();

    virtual QString format() const;
    void setFormat(const QString&);

    static QString stripFunctionName(const char*);

  protected:
    QString formattedString(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                            const char* function, const QString& category, const QString& message) const;

  private:
    static QByteArray qCleanupFuncinfo(const char*);

    QString m_format;
    mutable QReadWriteLock m_formatLock;
};

DCORE_END_NAMESPACE

#endif // ABSTRACTSTRINGAPPENDER_H
