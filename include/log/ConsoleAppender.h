// SPDX-FileCopyrightText: 2010 Boris Moiseev (cyberbobs at gmail dot com)
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONSOLEAPPENDER_H
#define CONSOLEAPPENDER_H

#include "CuteLogger_global.h"
#include <AbstractStringAppender.h>

DCORE_BEGIN_NAMESPACE

class CUTELOGGERSHARED_EXPORT ConsoleAppender : public AbstractStringAppender
{
  public:
    ConsoleAppender();
    virtual QString format() const;
    void ignoreEnvironmentPattern(bool ignore);

  protected:
    virtual void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                        const char* function, const QString& category, const QString& message);

  private:
    bool m_ignoreEnvPattern;
};

DCORE_END_NAMESPACE

#endif // CONSOLEAPPENDER_H
