// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QStringList>
#include <QDBusObjectPath>
#include <ddbusinterface.h>

using Dtk::Core::DDBusInterface;

class FakeDBusServiceParent: public QObject
{
    Q_OBJECT
public:
    explicit FakeDBusServiceParent(QObject *parent = nullptr);

    Q_PROPERTY(QList<QDBusObjectPath> objectPaths READ objectpaths NOTIFY objectpathsChanged);
    QList<QDBusObjectPath> objectpaths();
signals:
    void objectpathsChanged();
private:
    DDBusInterface *m_dDbusFakeDBusServiceInter;
};

class FakeDBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.FakeDBusService")
public:
    explicit FakeDBusService(QObject *parent = nullptr);
    ~FakeDBusService();

    Q_PROPERTY(QString strProperty READ strproperty)
    inline QString strproperty() { return m_strproperty; }

    Q_PROPERTY(QList<QDBusObjectPath> objectPaths READ objectpaths);
    inline QList<QDBusObjectPath> objectpaths() { return m_objectpaths; }

    static QString get_service() { return "org.deepin.FakeDBusService"; }
    static QString get_path() { return "/org/deepin/FakeDBusService"; }
    static QString get_interface() { return "org.deepin.FakeDBusService"; }

private:
    const QString m_service{"org.deepin.FakeDBusService"};
    const QString m_path{"/org/deepin/FakeDBusService"};
    QString m_strproperty;
    QList<QDBusObjectPath> m_objectpaths;
    void registerService();
    void unregisterService();
};
