// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPLUGIN_P_H
#define DPLUGIN_P_H

#include "dplugin.h"

#include <QJsonValueRef>

DCORE_BEGIN_NAMESPACE

class DPlugin;

class DPluginPrivate : public QObject
{
    Q_OBJECT;

public:
    explicit DPluginPrivate(const QString &name, DPlugin *parent);
    const QString name;

    static QList<std::shared_ptr<DPlugin>> searchPluginsInPath(const QString &pluginDirPath);
    static bool configCompatible(const QJsonValueRef &version);
    static std::shared_ptr<DPlugin> pluginFromJsonContent(const QByteArray &content);

    DPlugin *q_ptr;
    Q_DECLARE_PUBLIC(DPlugin);
};

DCORE_END_NAMESPACE

#endif
