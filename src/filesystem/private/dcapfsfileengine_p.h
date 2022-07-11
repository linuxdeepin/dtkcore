// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCAPFSFILEENGINE_P_H
#define DCAPFSFILEENGINE_P_H

#include <DObject>

#include <private/qfsfileengine_p.h>

DCORE_BEGIN_NAMESPACE

class DCapFSFileEngineHandler : public QAbstractFileEngineHandler
{
public:
    QAbstractFileEngine *create(const QString &fileName) const override;
};

class DCapFSFileEnginePrivate;
class DCapFSFileEngine : public QFSFileEngine, public DObject
{
    D_DECLARE_PRIVATE(DCapFSFileEngine);
public:
    DCapFSFileEngine(const QString &file);
    ~DCapFSFileEngine() override;

    bool open(QIODevice::OpenMode openMode) override;
    bool remove() override;
    bool copy(const QString &newName) override;
    bool rename(const QString &newName) override;
    bool renameOverwrite(const QString &newName) override;
    bool link(const QString &newName) override;
    bool mkdir(const QString &dirName, bool createParentDirectories) const override;
    bool rmdir(const QString &dirName, bool recurseParentDirectories) const override;
    FileFlags fileFlags(FileFlags type) const override;
    bool cloneTo(QAbstractFileEngine *target) override;
    bool setSize(qint64 size) override;
    QStringList entryList(QDir::Filters filters, const QStringList &filterNames) const override;
    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override;

    bool canReadWrite(const QString &path) const;
};

DCORE_END_NAMESPACE
#endif // DCAPFSFILEENGINE_P_H
