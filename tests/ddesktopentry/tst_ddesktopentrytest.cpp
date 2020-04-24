/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *               2019 Gary Wang
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#include <QString>
#include <QtTest>
#include <DDesktopEntry>

DCORE_USE_NAMESPACE

class DDesktopEntryTest : public QObject
{
    Q_OBJECT

public:
    DDesktopEntryTest();

private Q_SLOTS:
    void testCase_ParseFile();
};

DDesktopEntryTest::DDesktopEntryTest()
{
    //
}

const QString testFileContent = { QStringLiteral(R"desktop(# A. Example Desktop Entry File
[Desktop Entry]
Version=1.0
Type=Application
Name=Foo Viewer
Name[zh_CN]=福查看器
Comment=The best viewer for Foo objects available!
# Next line have an extra " character
Comment[zh_CN]=最棒的 "福 查看器！
TryExec=fooview
Exec=fooview %F
Icon=fooview
MimeType=image/x-foo;
Actions=Gallery;Create;

[Desktop Action Gallery]
Exec=fooview --gallery
Name=Browse Gallery

[Desktop Action Create]
Exec=fooview --create-new
Name=Create a new Foo!
Icon=fooview-new
)desktop") };

void DDesktopEntryTest::testCase_ParseFile()
{
    QTemporaryFile file("testReadXXXXXX.desktop");
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts << testFileContent;
    file.close();
    QVERIFY(QFile::exists(fileName));

    DDesktopEntry *desktopFile = new DDesktopEntry(fileName);
    QStringList allGroups = desktopFile->allGroups();
    QCOMPARE(allGroups.count(), 3);
    QVERIFY(allGroups.contains("Desktop Entry") &&
            allGroups.contains("Desktop Action Gallery") &&
            allGroups.contains("Desktop Action Create"));
    QCOMPARE(desktopFile->allGroups(true)[0], QStringLiteral("Desktop Entry"));
    QCOMPARE(desktopFile->localizedValue("Name", "zh_CN"), QStringLiteral("福查看器"));
    QCOMPARE(desktopFile->localizedValue("Name", "empty"), QStringLiteral("Foo Viewer"));
    QCOMPARE(desktopFile->keys(QStringLiteral("Desktop Entry")),
             QStringList({"Actions", "Comment", "Comment[zh_CN]", "Exec", "Icon", "MimeType", "Name", "Name[zh_CN]", "TryExec", "Type", "Version"}));

    {
        struct RestoreLocale {
            ~RestoreLocale() { QLocale::setDefault(QLocale::system()); }
        } restoreLocale;
        Q_UNUSED(restoreLocale);

        QLocale::setDefault(QLocale("zh_CN"));
        QCOMPARE(desktopFile->localizedValue("Name"), QStringLiteral("福查看器"));

        QLocale::setDefault(QLocale::c());
        QCOMPARE(desktopFile->localizedValue("Name"), QStringLiteral("Foo Viewer"));
    }

    QCOMPARE(desktopFile->stringValue("Name"), QStringLiteral("Foo Viewer"));
    QCOMPARE(desktopFile->setRawValue("Bar Viewer", "Name"), true);
    QCOMPARE(desktopFile->stringValue("Name"), QStringLiteral("Bar Viewer"));
    QCOMPARE(desktopFile->setLocalizedValue("霸查看器", "zh_CN", "Name"), true);
    QCOMPARE(desktopFile->localizedValue("Name", "zh_CN"), QStringLiteral("霸查看器"));
    QCOMPARE(desktopFile->contains("Semicolon"), false);
    QCOMPARE(desktopFile->setRawValue(";grp\\;2;grp3;", "Semicolon"), true);
    QCOMPARE(desktopFile->stringListValue("Semicolon"), QStringList({"", "grp;2", "grp3"}));
    QCOMPARE(desktopFile->contains("Semicolon"), true);
    QCOMPARE(desktopFile->removeEntry("Semicolon"), true);
    QCOMPARE(desktopFile->contains("Semicolon"), false);

    qDebug() << desktopFile->save();
    qDebug() << fileName;
}

QTEST_APPLESS_MAIN(DDesktopEntryTest)

#include "tst_ddesktopentrytest.moc"
