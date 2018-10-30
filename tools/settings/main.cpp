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

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDomDocument>

static QString CppTemplate =
    "#include <DSettings>\n"
    "\n"
    "void GenerateSettingTranslate()\n"
    "{\n"
    "%1"
    "}\n";

/*
 *  GVariant Type Name/Code      C++ Type Name          QVariant Type Name
 *  --------------------------------------------------------------------------
 *  boolean            b         bool                   QVariant::Bool
 *  byte               y         char                   QVariant::Char
 *  int16              n         int                    QVariant::Int
 *  uint16             q         unsigned int           QVariant::UInt
 *  int32              i         int                    QVariant::Int
 *  uint32             u         unsigned int           QVariant::UInt
 *  int64              x         long long              QVariant::LongLong
 *  uint64             t         unsigned long long     QVariant::ULongLong
 *  double             d         double                 QVariant::Double
 *  string             s         QString                QVariant::String
 *  string array*      as        QStringList            QVariant::StringList
 *  byte array         ay        QByteArray             QVariant::ByteArray
 *  dictionary         a{ss}     QVariantMap            QVariant::Map
*/

QString gsettings_type_from_QVarint(const QVariant::Type qtype)
{
    switch (qtype) {
    case QVariant::Bool:
        return "b";
    case QVariant::Int:
        return "i";
    case QVariant::UInt:
        return "u";
    case QVariant::LongLong:
        return "x";
    case QVariant::ULongLong:
        return "t";
    case QVariant::Double:
        return "d";
    case QVariant::String:
        return "s";
    case QVariant::StringList:
        return "as";
    case QVariant::ByteArray:
        return "ay";
    case QVariant::Map:
        return "a{ss}";
    default:
        return "";
    }
}

QString gsettings_value_from_QVarint(const QVariant value)
{
    switch (value.type()) {
    case QVariant::Bool:
        return value.toString();
    case QVariant::Int:
        return value.toString();
    case QVariant::UInt:
        return value.toString();
    case QVariant::LongLong:
        return value.toString();
    case QVariant::ULongLong:
        return value.toString();
    case QVariant::Double:
        return value.toString();
    case QVariant::String:
        return QString("\"%1\"").arg(value.toString());
    case QVariant::StringList:
        return value.toString();
    case QVariant::ByteArray:
        return value.toString();
    case QVariant::Map:
        return value.toString();
    default:
        return "";
    }
}


QJsonObject parseGSettingsMeta(const QString &jsonPath)
{
    QFile jsonFile(jsonPath);
    jsonFile.open(QIODevice::ReadOnly);
    auto jsonData = jsonFile.readAll();
    jsonFile.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    return jsonDoc.object().value("gsettings").toObject();
}

static bool writeGSettingXML(Dtk::Core::DSettings *settings,
                             QJsonObject gsettingsMeta,
                             const QString &xmlPath)
{
    QDomDocument document;

    QDomProcessingInstruction header = document.createProcessingInstruction("xml",
                                       "version=\"1.0\" encoding=\"utf-8\"");
    document.appendChild(header);

    QDomElement schemalist = document.createElement("schemalist");

    auto id = gsettingsMeta.value("id").toString();
    auto path = gsettingsMeta.value("path").toString();
    QDomElement schema = document.createElement("schema");
    schema.setAttribute("id", id);
    schema.setAttribute("path", path);

    for (QString key : settings->keys()) {
        auto codeKey = QString(key).replace(".", "-").replace("_", "-");
        auto value = settings->option(key)->value();
        auto gtype = gsettings_type_from_QVarint(value.type());
        if (gtype.isEmpty()) {
            qDebug() << "skip unsupported type:" << value.type() << key;
            continue;
        }

        QDomElement keyXml = document.createElement("key");
        keyXml.setAttribute("name", codeKey);
        keyXml.setAttribute("type", gtype);

        QString defaultData = gsettings_value_from_QVarint(value);
        QDomElement defaultEle = document.createElement("default");
        QDomCharacterData data = document.createTextNode(defaultData);
        defaultEle.appendChild(data);
        keyXml.appendChild(defaultEle);

        schema.appendChild(keyXml);
    }

    schemalist.appendChild(schema);
    document.appendChild(schemalist);

    QFile file(xmlPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QTextStream stream(&file);
    stream << document.toString();
    file.close();
    return true;
}

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

    QCommandLineOption gsettingsArg(QStringList() << "g" << "gsettings",
                                    QCoreApplication::tr("generate gsetting schema"),
                                    "xml-file");

    QCommandLineOption outputFileArg(QStringList() << "o" << "output",
                                     QCoreApplication::tr("Output cpp file"),
                                     "cpp-file");
    parser.addOption(gsettingsArg);
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


    if (parser.isSet(outputFileArg)) {
        QString outputCpp = CppTemplate.arg(cppCode);
        QFile outputFile(parser.value(outputFileArg));
        if (!outputFile.open(QIODevice::WriteOnly)) {
            qCritical() << "can not open output file!";
            exit(1);
        }
        outputFile.write(outputCpp.toUtf8());
        outputFile.close();
    }

    if (parser.isSet(gsettingsArg)) {
        QString outputXml = parser.value(gsettingsArg);
        writeGSettingXML(settings, parseGSettingsMeta(jsonFile), outputXml);
    }

    return 0;
}
