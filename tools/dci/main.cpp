/*
 * Copyright (C) 2021 UnionTech Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@uniontech.com>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

#include "dci/ddcifile.h"

static bool copyFilesToDci(DDciFile *dci, const QString &targetDir, const QString &sourceDir) {
    QDir dir(sourceDir);
    for (const auto &info : dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        const QString &newFile = QDir(targetDir).filePath(info.fileName());
        if (info.isDir()) {
            if (!dci->mkdir(newFile))
                return false;
            if (!copyFilesToDci(dci, newFile, info.absoluteFilePath()))
                return false;
        } else if (info.isSymLink()) {
            if (!dci->link(info.symLinkTarget(), newFile))
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

bool createTo(const QString &sourceDir, const QString &targetDir) {
    QFileInfo info(sourceDir);
    if (!info.isDir())
        return false;
    const auto newFile = QDir(targetDir).filePath(info.fileName() + ".dci");
    if (QFile::exists(newFile)) {
        return false;
    }

    DDciFile dci;
    if (!copyFilesToDci(&dci, "/", sourceDir))
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
            if (!QFile::link(dci->symlinkTarget(file), newFilePath))
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
