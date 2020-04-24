/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Chris Xiong <chirs241097@gmail.com>
 *
 * Maintainer: Chris Xiong <chirs241097@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util/dexportedinterface.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonValue>

#include <cmath>

//#define ALTERNATE_USAGE

DCORE_USE_NAMESPACE

class CustomInterface : public DUtil::DExportedInterface
{
    QVariant invoke(const QString &action, const QString &parameters) const
    {
        QJsonDocument d = QJsonDocument::fromJson(parameters.toUtf8());
        if (action == "pow") {
            return QVariant(pow(d["a"].toDouble(), d["b"].toDouble()));
        }
        return QVariant();
    }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QDBusConnection::sessionBus().registerService("com.deepin.ExpIntfTest");

#ifndef ALTERNATE_USAGE
    DUtil::DExportedInterface *ei = new DUtil::DExportedInterface();
    ei->registerAction("quit", "quit the application", [&app](QString)->QVariant {
        app.quit();
        return QVariant();
    });

    ei->registerAction("answer", "answer to the ultimate question of life, the universe, and everything",
                       [](QString)->QVariant {return QVariant(42);});

    ei->registerAction("sum", "returns the sum of two integers", [](QString p)->QVariant {
        QJsonDocument d = QJsonDocument::fromJson(p.toUtf8());
        return QVariant(d["a"].toInt() + d["b"].toInt());
    });
#else
    CustomInterface *cei = new CustomInterface();
    cei->registerAction("pow", "raise a number to a power");
#endif

    return app.exec();
}
