// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OUTPUTDEBUGAPPENDER_H
#define OUTPUTDEBUGAPPENDER_H

#include "dtkcore_global.h"
#include <AbstractStringAppender.h>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT OutputDebugAppender : public AbstractStringAppender
{
  protected:
    virtual void append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                        const char *func, const QString &category, const QString &msg);
};

DCORE_END_NAMESPACE

#endif // OUTPUTDEBUGAPPENDER_H
