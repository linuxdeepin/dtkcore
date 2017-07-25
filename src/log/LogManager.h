#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QtCore>

#include "CuteLogger_global.h"

DCORE_BEGIN_NAMESPACE

class ConsoleAppender;
class RollingFileAppender;

class DLogManager
{
public:
    static void registerConsoleAppender();
    static void registerFileAppender();
    static QString getlogFilePath();
    static void setLogFormat(const QString& format);

Q_SIGNALS:

public Q_SLOTS:

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
