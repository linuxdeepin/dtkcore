// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

#include <unistd.h>

#include "dci/ddcifile.h"

static QString getSymLinkTarget(const QString &file, const QString &originPath) {
    char target[512] = {0};
    const auto ret = readlink(file.toLocal8Bit().constData(), target, 511);
    if (ret <= 0)
        return QString();
    QString tar = QByteArray(target, ret);
    QString path = originPath;
    if (originPath.endsWith('/'))
        path = path.left(path.length() - 1);
    if (tar.startsWith(path)) {
        tar = tar.remove(0, path.length());
    }
    return tar;
}

static bool copyFilesToDci(DDciFile *dci, const QString &targetDir, const QString &sourceDir, const QString &originPath) {
    QDir dir(sourceDir);
    for (const auto &info : dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        const QString &newFile = QDir(targetDir).filePath(info.fileName());
        if (info.isDir()) {
            if (!dci->mkdir(newFile))
                return false;
            if (!copyFilesToDci(dci, newFile, info.absoluteFilePath(), originPath))
                return false;
        } else if (info.isSymLink()) {
            if (!dci->link(getSymLinkTarget(info.absoluteFilePath(), originPath), newFile))
                return false;
        } else if (info.isFile()) {
            QFile file(info.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly))
                return false;
            if (!dci->writeFile(newFile, file.readAll()))
                return false;
        }
    }

    return true;
}

QString cleanPath(const QString &path) {
    return path.size() < 2 || !path.endsWith(QDir::separator()) ? path : path.chopped(1);
}

bool createTo(const QString &sourceDir, const QString &targetDir) {
    QFileInfo info(cleanPath(sourceDir));
    if (!info.isDir())
        return false;

    const QString &iconName = info.fileName();
    if (iconName.isEmpty()) {
        printf("The icon name is not correctly resolved in the source path: \"%s\".\n", qPrintable(sourceDir));
        return false;
    }

    const auto newFile = QDir(targetDir).filePath(iconName + ".dci");
    if (QFile::exists(newFile)) {
        printf("The path \"%s\" already exists.\n", qPrintable(newFile));
        return false;
    }

    DDciFile dci;
    if (!copyFilesToDci(&dci, "/", sourceDir, sourceDir))
        return false;

    return dci.writeToFile(newFile);
}

static bool copyFilesFromDci(const DDciFile *dci, const QString &targetDir, const QString &sourceDir) {
    QDir target(targetDir);
    for (const QString &file : dci->list(sourceDir)) {
        const QString &newFileName = QFileInfo(file).fileName();
        const QString &newFilePath = target.filePath(newFileName);
        const auto &type = dci->type(file);
        if (type == DDciFile::Directory) {
            if (!target.mkdir(newFileName))
                return false;
            if (!copyFilesFromDci(dci, newFilePath, file))
                return false;
        } else if (type == DDciFile::File) {
            QFile newFile(newFilePath);
            if (!newFile.open(QIODevice::WriteOnly))
                return false;
            const auto &data = dci->dataRef(file);
            if (newFile.write(data) != data.size())
                return false;
        } else if (type == DDciFile::Symlink) {
            if (!QFile::link(dci->symlinkTarget(file, true), newFilePath))
                return false;
        } else {
            return false;
        }
    }

    return true;
}

bool exportTo(const QString &dciFile, const QString &targetDir) {
    QFileInfo info(dciFile);
    if (!info.isFile() || info.suffix() != "dci")
        return false;
    const QString &newDir = QDir(targetDir).filePath(info.baseName());
    if (QDir(newDir).exists())
        return false;
    if (!QDir::current().mkdir(newDir))
        return false;

    DDciFile dci(dciFile);
    if (!dci.isValid())
        return false;

    return copyFilesFromDci(&dci, newDir, "/");
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setOrganizationName("zccrs");
    a.setApplicationName("dci");
    a.setApplicationVersion("0.0.1");

    QCommandLineParser commandParser;
    commandParser.setApplicationDescription("DCI tool is a command tool that automatically packs and unpacks DCI directories.\n"
                                            "If you have created an icon directory according to the correct DCI directory specification,\n"
                                            "you can easily make the DCI icons with this tool.\n"
                                            "The commands of DCI tools can be expressed as follows:\n"
                                            "\t dci --create [target file path] [source directory path]\n"
                                            "\t dci --export [target directory path] [source file path]\n"
                                            "For example, the tool is used in the following ways: \n"
                                            "\t dci --create ~/Desktop ~/Desktop/action_add\n"
                                            "\t dci --export ~/Desktop ~/Desktop/action_add.dci\n");

    auto options = QList<QCommandLineOption> {
        QCommandLineOption("create", "Create the new dci files by the directorys", "targetDirectiry"),
        QCommandLineOption("export", "Export the dci files to the directorys", "targetDirectory"),
    };
    commandParser.addOptions(options);
    commandParser.addPositionalArgument("sources", "The directorys of create or the dci files of export",
                                        "[dir1 dir2...]/[file1 file2...]");
    commandParser.addHelpOption();
    commandParser.addVersionOption();
    commandParser.process(a);

    if (commandParser.isSet(options.at(0))) {
        for (const QString &dir : commandParser.positionalArguments()) {
            if (!createTo(dir, commandParser.value(options.at(0)))) {
                printf("Failed on create dci file for \"%s\"\n", qPrintable(dir));
            }
        }
    } else if (commandParser.isSet(options.at(1))) {
        for (const QString &dci : commandParser.positionalArguments()) {
            if (!exportTo(dci, commandParser.value(options.at(1)))) {
                printf("Failed on export the \"%s\" dci file\n", qPrintable(dci));
            }
        }
    } else {
        commandParser.showHelp(-1);
    }

    return 0;
}
