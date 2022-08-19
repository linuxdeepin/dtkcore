// SPDX-FileCopyrightText: 2010 Karl-Heinz Reichel (khreichel at googlemail dot com)
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OUTPUTDEBUGAPPENDER_H
#define OUTPUTDEBUGAPPENDER_H

#include "CuteLogger_global.h"
#include <AbstractStringAppender.h>

DCORE_BEGIN_NAMESPACE

class CUTELOGGERSHARED_EXPORT OutputDebugAppender : public AbstractStringAppender
{
  protected:
    virtual void append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                        const char* function, const QString& category, const QString& message);
};

DCORE_END_NAMESPACE

#endif // OUTPUTDEBUGAPPENDER_H
