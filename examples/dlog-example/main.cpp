// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DtkCore/DLog"
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QDebug>
DCORE_USE_NAMESPACE

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
#ifdef Q_OS_LINUX
    DLogManager::registerJournalAppender();
#endif
    DLogManager::registerConsoleAppender();
    dDebug() << "this is a debug message";
    return app.exec();
}
