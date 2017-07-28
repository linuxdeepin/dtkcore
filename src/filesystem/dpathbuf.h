/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QDir>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DPathBuf
{
public:
    DPathBuf(const QString &path)
    {
        m_path = QDir(path).absolutePath();
    }

    DPathBuf operator/(const QString &p) const
    {
        return DPathBuf(m_path + "/" + p);
    }

    DPathBuf &operator/=(const QString &p)
    {
        return join(p);
    }

    DPathBuf operator/(const char *p) const
    {
        return operator /(QString(p));
    }

    DPathBuf &operator/=(const char *p)
    {
        return operator /=(QString(p));
    }

    DPathBuf &join(const QString &p)
    {
        m_path += "/" + p;
        m_path = QDir(m_path).absolutePath();
        return *this;
    }

    QString toString() const
    {
        return QDir::toNativeSeparators(m_path);
    }

private:
    QString m_path;
};

DCORE_END_NAMESPACE
