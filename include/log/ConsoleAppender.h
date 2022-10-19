// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONSOLEAPPENDER_H
#define CONSOLEAPPENDER_H

#include "dtkcore_global.h"
#include <AbstractStringAppender.h>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT ConsoleAppender : public AbstractStringAppender
{
public:
    ConsoleAppender();
    virtual QString format() const;
    void ignoreEnvironmentPattern(bool ignore);

protected:
    virtual void append(const QDateTime &time, Logger::LogLevel level, const char *file, int line,
                      const char *func, const QString &category, const QString &msg);

private:
    bool m_ignoreEnvPattern;
};

DCORE_END_NAMESPACE

#endif // CONSOLEAPPENDER_H
