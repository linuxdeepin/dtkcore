/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QObject>
#include <QPointer>

#include "dsettingsoption.h"

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettingsGroupPrivate;
class DSettingsGroup : public QObject
{
    Q_OBJECT
public:
    explicit DSettingsGroup(QObject *parent = 0);
    ~DSettingsGroup();

    QPointer<DSettingsGroup> parentGroup() const;
    void setParentGroup(QPointer<DSettingsGroup> parentGroup);

    QString key() const;
    QString name() const;

    QList<QPointer<DSettingsGroup> > childGroups() const;
    QList<QPointer<DSettingsOption> > childOptions() const;
    QList<QPointer<DSettingsOption> > options() const;

    static QPointer<DSettingsGroup> fromJson(const QString &prefixKey, const QJsonObject &group);
Q_SIGNALS:

public Q_SLOTS:

private:
    void parseJson(const QString &prefixKey, const QJsonObject &group);

    QScopedPointer<DSettingsGroupPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettingsGroup)
};

typedef QPointer<DSettingsGroup> GroupPtr;

DCORE_END_NAMESPACE
