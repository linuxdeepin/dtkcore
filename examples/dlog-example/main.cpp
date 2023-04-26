// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DLog>
#include <QCoreApplication>
DCORE_USE_NAMESPACE

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
#ifdef Q_OS_LINUX
    DLogManager::registerJournalAppender();
#endif
    DLogManager::registerConsoleAppender();
    dDebug() << "this is a debug message";
    qWarning() << "it is a Warning";
    qInfo() << "it is an Info";
    qCritical() << "it is a tracing";
    return app.exec();
}
