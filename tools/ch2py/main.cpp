// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include "dpinyin.h"
#include <QCommandLineParser>

DCORE_USE_NAMESPACE

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("ch2py");
    a.setApplicationVersion("0.0.1");

    QCommandLineParser cp;
    cp.setApplicationDescription("ch2py tool is a command tool that convert Chinese words to Pinyin.\n"
                                            "The commands of DCI tools can be expressed as follows:\n"
                                            "\t ch2py [chinese words]\n"
                                            "\t ch2py --tonestyle notones [chinese words]\n"
                                            "\t ch2py --letters [chinese words]\n"
                                            );


    QCommandLineOption tonestyle = QCommandLineOption(QStringList() << "s" << "tonestyle",
                                "tone style value can be \"notones\",\"tones\",\"numtones\"",
                                "tonestyle",
                                "tones");
    QCommandLineOption letters = QCommandLineOption(QStringList() << "l" << "letters",
                                "convert Chinese words to Pinyin first letters");
    cp.addOption(tonestyle);
    cp.addOption(letters);
    cp.addPositionalArgument("words", "words to be converted to pinyin");
    cp.addHelpOption();

    cp.process(a);

    QString words = cp.positionalArguments().join(" ");

    if (words.isEmpty()) {
        cp.showHelp();
    }

    QString tones = cp.value("tonestyle");

    ToneStyle ts = TS_Tone;
    if (!tones.compare("notones"))
        ts = TS_NoneTone;
    else if (!tones.compare("numtones"))
        ts = TS_ToneNum;

    if (cp.isSet(letters)) {
        const auto &ls = firstLetters(words);
        printf("%s\n", qPrintable(ls.join("\n")));
        printf("total size: %d\n", ls.size());
        return 0;
    }

    const auto &py = pinyin(words, ts);
    printf("%s\n", qPrintable(py.join("\n")));
    printf("total size: %d\n", py.size());

    return 0;//a.exec();
}
