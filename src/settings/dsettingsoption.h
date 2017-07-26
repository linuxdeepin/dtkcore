/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QVariant>
#include <QObject>
#include <QPointer>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettingsGroup;
class DSettingsOptionPrivate;
class DSettingsOption : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit DSettingsOption(QObject *parent = 0);
    ~DSettingsOption();

    QPointer<DSettingsGroup> parentGroup() const;
    void setParentGroup(QPointer<DSettingsGroup> parentGroup);

    QString key() const;
    QString name() const;
    bool canReset() const;
    QVariant defaultValue() const;
    QVariant value() const;
    QVariant data(const QString &dataType) const;

    QString viewType() const;
    bool isHidden() const;

    static QPointer<DSettingsOption> fromJson(const QString &prefixKey, const QJsonObject &group);
Q_SIGNALS:
    void valueChanged(QVariant value);
    void dataChanged(const QString &dataType, QVariant value);

public Q_SLOTS:
    void setValue(QVariant value);
    void setData(const QString &dataType, QVariant value);

private:
    void parseJson(const QString &prefixKey, const QJsonObject &option);

    QScopedPointer<DSettingsOptionPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettingsOption)
};

typedef QPointer<DSettingsOption> OptionPtr;

DCORE_END_NAMESPACE
