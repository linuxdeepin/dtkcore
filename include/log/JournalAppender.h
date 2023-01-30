// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef JOURNALAPPENDER_H
#define JOURNALAPPENDER_H

#include "dtkcore_global.h"
#include <AbstractAppender.h>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT JournalAppender : public AbstractAppender
{
public:
    JournalAppender()
        : AbstractAppender()
    {
    }
    ~JournalAppender() override = default;
    using JournalPriority = int;

protected:
    virtual void append(const QDateTime &time,
                        Logger::LogLevel level,
                        const char *file,
                        int line,
                        const char *func,
                        const QString &category,
                        const QString &msg);

private:
    bool m_ignoreEnvPattern;
};

DCORE_END_NAMESPACE

#endif  // JOURNALAPPENDER_H
