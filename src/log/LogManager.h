/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QtCore>

#include "CuteLogger_global.h"

DCORE_BEGIN_NAMESPACE

class ConsoleAppender;
class RollingFileAppender;

class LIBDTKCORESHARED_EXPORT DLogManager
{
public:
    static void registerConsoleAppender();
    static void registerFileAppender();

    static QString getlogFilePath();

    /*!
     * \brief setlogFilePath will change log file path of registerFileAppender
     * \param logFilePath is the full path of file appender log
     */
    static void setlogFilePath(const QString& logFilePath);

    static void setLogFormat(const QString& format);

private:
    QString m_format;
    QString m_logPath;
    ConsoleAppender* m_consoleAppender;
    RollingFileAppender* m_rollingFileAppender;

    void initConsoleAppender();
    void initRollingFileAppender();
    QString joinPath(const QString& path, const QString& fileName);

    inline static DLogManager* instance(){
        static DLogManager instance;
        return &instance;
    }
    explicit DLogManager();
    ~DLogManager();
    DLogManager(const DLogManager &);
    DLogManager & operator = (const DLogManager &);
};

DCORE_END_NAMESPACE

#endif // LOGMANAGER_H
