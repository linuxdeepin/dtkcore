// SPDX-FileCopyrightText: 2016 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QTimer>
#include <QThread>
#include <QMetaObject>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <type_traits>
#include <cstring>

namespace DUtil
{

template <typename Func1>
inline void TimerSingleShot(int msec,  Func1 slot)
{
#if QT_VERSION >= 0x050500
    QTimer::singleShot(msec, slot);
#else
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    timer->setInterval(msec);
    timer->moveToThread(qApp->thread());
    QObject::connect(timer, &QTimer::timeout, slot);
    QObject::connect(timer, &QTimer::timeout, timer, &QTimer::deleteLater);
    if (QThread::currentThread() == qApp->thread()) { timer->start(); }
    else { QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection); }
#endif
}

template <class T>
void SecureErase(T *p, size_t size)
{
    static_assert(std::is_standard_layout<T>::value && std::is_trivially_destructible<T>::value,
                  "try to erase content of raw pointer, but type T isn't suitable");

    std::memset(p, 0, size);
}

template <class T>
void SecureErase(T &obj)
{
    static_assert(std::is_default_constructible<typename T::value_type>::value,
                  "container's value type must have a default constructor.");

    for (typename T::iterator i = obj.begin(); i != obj.end(); ++i) {
        *i = typename T::value_type{};
    }
}

inline QString escapeToObjectPath(const QByteArray &str) noexcept
{
    if (str.isEmpty()) {
        return QStringLiteral("_");
    }

    QString ret;
    ret.reserve(str.size() * 3);

    for (qsizetype i = 0; i < str.size(); ++i) {
        auto byte = static_cast<unsigned char>(str.at(i));
        if (std::isalnum(byte) != 0 || byte == '/') {
            ret.append(QChar::fromLatin1(byte));
        } else {
            // TODO: a valid dbus object path component only allows "[A-Z][a-z][0-9]_"
            // for compatibility with existing applications, we escape all unicode to avoid breakage
            // but we should consider to drop this compatibility hack in the future.
            ret.append(u'_');
            ret.append(QString::number(byte, 16).rightJustified(2, u'0').toLower());
        }
    }

    ret.shrink_to_fit();
    return ret;
}

inline QString escapeToObjectPath(const QString &str) noexcept
{
    return escapeToObjectPath(str.toUtf8());
}

inline QString unescapeFromObjectPath(const QString &str) noexcept
{
    QByteArray ret;
    ret.reserve(str.length());

    for (qsizetype i = 0; i < str.length();) {
        if (i <= str.length() - 3 && str.at(i) == u'_') {
            bool ok{false};
            auto byte = static_cast<unsigned char>(str.mid(i + 1, 2).toUShort(&ok, 16));
            if (ok) {
                ret.append(static_cast<char>(byte));
                i += 3;
                continue;
            }
        }

        ret.append(str.at(i).toLatin1());
        ++i;
    }

    return QString::fromUtf8(ret);
}

inline QString getAppIdFromAbsolutePath(const QString &path)
{
    static QString desktopSuffix{u8".desktop"};
    const auto &appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    if (!path.endsWith(desktopSuffix) ||
        !std::any_of(appDirs.cbegin(), appDirs.constEnd(), [&path](const QString &dir) { return path.startsWith(dir); })) {
        return {};
    }

    auto tmp = path.chopped(desktopSuffix.size());
    auto components = tmp.split(QDir::separator(), Qt::SkipEmptyParts);
    auto location = std::find(components.cbegin(), components.cend(), "applications");
    if (location == components.cend()) {
        return {};
    }

    auto appId = QStringList{location + 1, components.cend()}.join('-');
    return appId;
}

inline QStringList getAbsolutePathFromAppId(const QString &appId)
{
    auto components = appId.split('-', Qt::SkipEmptyParts);
    auto appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    QStringList ret;
    for (const auto &dirPath : appDirs) {
        auto currentDir = dirPath;
        for (auto it = components.cbegin(); it != components.cend(); ++it) {
            auto currentName = QStringList{it, components.cend()}.join('-') + QString{".desktop"};
            QDir dir{currentDir};
            if (dir.exists(currentName)) {
                ret.append(dir.filePath(currentName));
            }

            currentDir.append(QDir::separator() + *it);
        }
    }

    return ret;
}
}
