// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLUGIN_H
#define DPLUGIN_H

#include "dtkcore_global.h"

#include <QLibrary>

#include <memory>

DCORE_BEGIN_NAMESPACE

class DPluginPrivate;

class DPlugin : public QObject
{
    Q_OBJECT;

public:
    static QList<std::shared_ptr<DPlugin>> listPlugins(const QStringList &extraLocations = {});

    const QScopedPointer<QLibrary> library;
    QString name();

private:
    DPlugin(const QString &name, const QString &filename, const QString &version, QObject *parent = nullptr);

    QScopedPointer<DPluginPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DPlugin);
};

DCORE_END_NAMESPACE

#endif
