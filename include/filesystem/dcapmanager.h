// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCAPMANAGER_H
#define DCAPMANAGER_H

#include <DObject>

#include <QObject>

DCORE_BEGIN_NAMESPACE
class DCapManagerPrivate;
class DCapManager : public QObject, public DObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DCapManager)
    D_DECLARE_PRIVATE(DCapManager)
public:
    static DCapManager *instance();
    static void registerFileEngine();
    static void unregisterFileEngine();

    void appendPath(const QString &path);
    void appendPaths(const QStringList &pathList);

    void removePath(const QString &path);
    void removePaths(const QStringList &paths);

    QStringList paths() const;

protected:
    explicit DCapManager();
};

DCORE_END_NAMESPACE
#endif // DCAPMANAGER_H
