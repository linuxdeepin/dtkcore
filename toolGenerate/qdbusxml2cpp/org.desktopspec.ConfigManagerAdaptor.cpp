/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dtkcore/src/dbus/org.desktopspec.ConfigManager.xml -a ./dtkcore/toolGenerate/qdbusxml2cpp/org.desktopspec.ConfigManagerAdaptor -i ./dtkcore/toolGenerate/qdbusxml2cpp/org.desktopspec.ConfigManager.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "./dtkcore/toolGenerate/qdbusxml2cpp/org.desktopspec.ConfigManagerAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ConfigManagerAdaptor
 */

ConfigManagerAdaptor::ConfigManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ConfigManagerAdaptor::~ConfigManagerAdaptor()
{
    // destructor
}

QDBusObjectPath ConfigManagerAdaptor::acquireManager(const QString &appid, const QString &name, const QString &subpath)
{
    // handle method call org.desktopspec.ConfigManager.acquireManager
    QDBusObjectPath path;
    QMetaObject::invokeMethod(parent(), "acquireManager", Q_RETURN_ARG(QDBusObjectPath, path), Q_ARG(QString, appid), Q_ARG(QString, name), Q_ARG(QString, subpath));
    return path;
}

void ConfigManagerAdaptor::sync(const QString &path)
{
    // handle method call org.desktopspec.ConfigManager.sync
    QMetaObject::invokeMethod(parent(), "sync", Q_ARG(QString, path));
}

void ConfigManagerAdaptor::update(const QString &path)
{
    // handle method call org.desktopspec.ConfigManager.update
    QMetaObject::invokeMethod(parent(), "update", Q_ARG(QString, path));
}

