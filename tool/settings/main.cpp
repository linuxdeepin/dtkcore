/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>

#include <iostream>

#include <QDebug>
#include <QFile>
#include <QCommandLineParser>

#include "settings/dsettings.h"
#include "settings/dsettingsgroup.h"
#include "settings/dsettingsoption.h"


static QString CppTemplate =
    "#include <DSettings>\n"
    "\n"
    "void GenerateSettingTranslate()\n"
    "{\n"
    "%1"
    "}\n";

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dtk-settings-tools");
    app.setApplicationVersion("0.1.2");

    QCommandLineParser parser;
    parser.setApplicationDescription("Generate translation of dtksetting.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption outputFileArg(QStringList() << "o" << "output",
                                     QCoreApplication::tr("Output cpp file"),
                                     "cpp-file");
    parser.addOption(outputFileArg);
    parser.addPositionalArgument("json-file", QCoreApplication::tr("Json file description config"));
    parser.process(app);

    if (0 == (parser.optionNames().length() + parser.positionalArguments().length())) {
        parser.showHelp(0);
    }

    auto jsonFile = parser.positionalArguments().value(0);
    auto settings = Dtk::Core::DSettings::fromJsonFile(jsonFile);

    QMap<QString, QString> transtaleMaps;

//    qDebug() << settings->groupKeys();
    for (QString groupKey : settings->groupKeys()) {
        auto codeKey = QString(groupKey).replace(".", "_");
        auto group = settings->group(groupKey);
//        qDebug() << codeKey << group->name();
        // add Name
        if (!group->name().isEmpty()) {
            transtaleMaps.insert("group_" + codeKey + "Name", group->name());
        }

        // TODO: only two level
        for (auto childGroup : group->childGroups()) {
            auto codeKey = childGroup->key().replace(".", "_");
//            qDebug() << codeKey << childGroup->name();
            // add Name
            if (!childGroup->name().isEmpty()) {
                transtaleMaps.insert("group_" + codeKey + "Name", childGroup->name());
            }
        }
    }

    for (QString key : settings->keys()) {
        auto codeKey = QString(key).replace(".", "_");
        auto opt = settings->option(key);

        // add Name
        if (!opt->name().isEmpty()) {
            transtaleMaps.insert(codeKey + "Name", opt->name());
        }

        // add text
        if (!opt->data("text").toString().isEmpty()) {
            transtaleMaps.insert(codeKey + "Text", opt->data("text").toString());
        }

        // add items
        if (!opt->data("items").toStringList().isEmpty()) {
            auto items = opt->data("items").toStringList();
            for (int i = 0; i < items.length(); ++i) {
                transtaleMaps.insert(codeKey + QString("Text%1").arg(i), items.value(i));
            }
        }
    }

    transtaleMaps.insert("reset_button_name", "Restore Defaults");

    QString cppCode;
    for (auto key : transtaleMaps.keys()) {
        auto stringCode = QString("    auto %1 = QObject::tr(\"%2\");\n").arg(key).arg(transtaleMaps.value(key));
        cppCode.append(stringCode);
    }

    QString outputCpp = CppTemplate.arg(cppCode);

    if (parser.isSet(outputFileArg)) {
        QFile outputFile(parser.value(outputFileArg));
        if (!outputFile.open(QIODevice::WriteOnly)) {
            qCritical() << "can not open output file!";
            exit(1);
        }
        outputFile.write(outputCpp.toUtf8());
        outputFile.close();
    } else {
        std::cout << outputCpp.toStdString();
    }
    return 0;
}

