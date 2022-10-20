// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DERROR_H
#define DERROR_H
#include "dtkcore_global.h"
#include <QString>
#include <QDebug>

DCORE_BEGIN_NAMESPACE

class Derror
{
public:
    Derror() noexcept
        : m_code(-1)
        , m_msg()
    {
    }

    Derror(const Derror &e) noexcept
        : m_code(e.m_code)
        , m_msg(e.m_msg)
    {
    }

    Derror(Derror &&e) noexcept
        : m_code(e.m_code)
        , m_msg(std::move(e).m_msg)
    {
    }

    Derror(qint64 code, const QString &msg) noexcept
        : m_code(code)
        , m_msg(msg)
    {
    }

    Derror(qint64 code, QString &&msg) noexcept
        : m_code(code)
        , m_msg(std::move(msg))
    {
    }

    Derror &operator=(const Derror &e)
    {
        m_code = e.m_code;
        m_msg = e.m_msg;
        return *this;
    }

    Derror &operator=(Derror &&e)
    {
        m_code = e.m_code;
        m_msg = std::move(e).m_msg;
        return *this;
    }

    ~Derror() = default;

    qint64 getErrorCode() const noexcept { return m_code; }

    void setErrorCode(qint64 code) &noexcept { m_code = code; }

    const QString &getErrorMessage() const & { return m_msg; }
    QString getErrorMessage() const && { return std::move(m_msg); }

    void setErrorMessage(const QString &msg) & { m_msg = msg; }

    friend bool operator==(const Derror &x, const Derror &y) noexcept { return x.m_code == y.m_code and x.m_msg == y.m_msg; }
    friend bool operator!=(const Derror &x, const Derror &y) noexcept { return !(x == y); }

    friend QDebug operator<<(QDebug debug, const Derror &e)
    {
        debug << "Error Code:" << e.m_code << "Message:" << e.m_msg;
        return debug;
    }

private:
    qint64 m_code;
    QString m_msg;
};
DCORE_END_NAMESPACE
#endif
