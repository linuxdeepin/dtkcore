// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dplugin.h"
#include "dplugin_p.h"

#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

DCORE_BEGIN_NAMESPACE

QList<std::shared_ptr<DPlugin>> DPlugin::listPlugins(const QStringList &extraDirs)
{
    auto pluginDirs = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    for (auto &pluginDir : pluginDirs) {
        pluginDir = QStringList{pluginDir, "plugins"}.join(QDir::separator());
    }

    pluginDirs.append(extraDirs);

    qDebug() << "Searching dplugins in" << pluginDirs;

    QList<std::shared_ptr<DPlugin>> plugins{};

    for (const auto &pluginDir : pluginDirs) {
        plugins << DPluginPrivate::searchPluginsInPath(pluginDir);
    }

    return plugins;
}

QList<std::shared_ptr<DPlugin>> DPluginPrivate::searchPluginsInPath(const QString &pluginDirPath)
{
    auto const pluginDir = QDir(pluginDirPath);
    auto const entryList = pluginDir.entryList();

    QList<std::shared_ptr<DPlugin>> result{};

    for (auto const &entry : entryList) {
        auto filePath = QStringList{pluginDirPath, entry}.join(QDir::separator());
        auto fileInfo = QFileInfo(filePath);

        // NOTE(black_desk): we do not load a large file (>4KB) into memory
        if (!fileInfo.isFile() || fileInfo.size() > (1 << 11)) {
            continue;
        }

        QFile file(filePath);
        file.open(QIODevice::ReadOnly);
        auto content = file.readAll();

        auto plugin = DPluginPrivate::pluginFromJsonContent(content);
        if (!plugin) {
            qWarning() << "Ignore invalid plugin config:" << filePath;
            continue;
        }
        result << plugin;
    }

    return result;
}

std::shared_ptr<DPlugin> DPluginPrivate::pluginFromJsonContent(const QByteArray &content)
{
    QJsonParseError err;
    auto jsonDoc = QJsonDocument::fromJson(content, &err);
    if (err.error || !jsonDoc.isObject()) {
        return nullptr;
    }

    auto config = jsonDoc.object();
    if (!DPluginPrivate::configCompatible(config["pluginConfigVersion"])) {
        return nullptr;
    }

    auto nameVar = config["pluginName"];
    if (!nameVar.isString()) {
        return nullptr;
    }

    auto name = nameVar.toString();

    auto versionVar = config["pluginVersion"];
    if (!nameVar.isString()) {
        return nullptr;
    }

    auto version = versionVar.toString();

    auto filenameVar = config["pluginFilename"];
    if (!filenameVar.isString()) {
        return nullptr;
    }

    auto filename = filenameVar.toString();

    return std::shared_ptr<DPlugin>(new DPlugin(name, filename, version));
}

DPlugin::DPlugin(const QString &name, const QString &filename, const QString &version, QObject *parent)
    : QObject(parent)
    , library(new QLibrary(filename, version))
    , d_ptr(new DPluginPrivate(name, this))
{
}

QString DPlugin::name()
{
    Q_D(DPlugin);
    return d->name;
}

bool DPluginPrivate::configCompatible(const QJsonValueRef &version)
{
    return version.isString() && version.toString() == "1";
}

DPluginPrivate::DPluginPrivate(const QString &name, DPlugin *parent)
    : QObject(parent)
    , name(name)
{
}

DCORE_END_NAMESPACE
