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
#include <QScopedPointer>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettingsBackend;
class DSettingsOption;
class DSettingsGroup;
class DSettingsPrivate;
class DSettings : public QObject
{
    Q_OBJECT
public:
    explicit DSettings(QObject *parent = 0);
    ~DSettings();

    void setBackend(DSettingsBackend *backend = nullptr);

    static QPointer<DSettings> fromJson(const QByteArray &json);
    static QPointer<DSettings> fromJsonFile(const QString &filepath);

    QStringList keys() const;
    QList<QPointer<DSettingsOption>> options() const;
    QPointer<DSettingsOption> option(const QString &key) const;
    QVariant value(const QString &key) const;

    QStringList groupKeys() const;
    QList<QPointer<DSettingsGroup>> groups() const;
    QPointer<DSettingsGroup> group(const QString &key) const;

    QVariant getOption(const QString &key) const;

Q_SIGNALS:
    void valueChanged(const QString &key, const QVariant &value);

public Q_SLOTS:
    //!
    //! \brief sync
    //! WARNING: sync will block
    void sync() ;

    void setOption(const QString &key, const QVariant &value);
    void reset() ;

private:
    void parseJson(const QByteArray &json);
    void loadValue();

    QScopedPointer<DSettingsPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettings)
};

DCORE_END_NAMESPACE
