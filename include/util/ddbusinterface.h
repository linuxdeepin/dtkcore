// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include <QDBusAbstractInterface>

class DDBusInterfacePrivate;

class DDBusInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    explicit DDBusInterface(const QString &service, const QString &path, const QString &interface = QString(),
                              const QDBusConnection &connection = QDBusConnection::sessionBus(), QObject *parent = nullptr);
    virtual ~DDBusInterface() override;

    bool serviceValid() const;
    QString suffix() const;
    void setSuffix(const QString &suffix);

    QVariant property(const char *propname);
    void setProperty(const char *propname, const QVariant &value);

Q_SIGNALS:
    void serviceValidChanged(const bool valid) const;

private:
    QScopedPointer<DDBusInterfacePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DDBusInterface)
    Q_DISABLE_COPY(DDBusInterface)
};

