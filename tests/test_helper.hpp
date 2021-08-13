/*
 * Copyright (C) 2021 Uniontech Technology Co., Ltd.
 *
 * Author:     yeshanshan <yeshanshan@live.com>
 *
 * Maintainer: yeshanshan <yeshanshan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <QDir>

class EnvGuard {
public:
    void set(const char *name, const QByteArray &value)
    {
        m_name = name;
        m_originValue = qgetenv(m_name);
        qputenv(m_name, value);

        if (!QDir(value).exists()) {
            QDir().mkpath(value);
        }
    }
    void restore()
    {
        qputenv(m_name, m_originValue);
    }
    QString value()
    {
        return qgetenv(m_name);
    }
private:
    QByteArray m_originValue;
    const char* m_name = nullptr;
};


class FileCopyGuard {
public:
    FileCopyGuard(const QString &source, const QString &target)
        : m_target(target)
    {
        if (!QFile::exists(QFileInfo(target).path()))
            QDir().mkpath(QFileInfo(target).path());
        QFile::copy(source, target);
    }
    ~FileCopyGuard(){ QFile::remove(m_target); }
private:
    QString m_target;
};
