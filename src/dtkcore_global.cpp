#include "dtkcore_global.h"
#include <QDebug>
#include <QFileInfo>

#if (!defined DTK_VERSION) || (!defined DTK_VERSION_STR)
#error "DTK_VERSION or DTK_VERSION_STR not defined!"
#endif

void doubleLoadCheck()
{
    QFile f("/proc/self/maps");
    if (!f.open(QIODevice::ReadOnly))
        qFatal("%s", f.errorString().toLocal8Bit().data());

    const QByteArray &data = f.readAll();
    QTextStream ts(data);
    QString modulePath;
    while (Q_UNLIKELY(!ts.atEnd())) {
        const QString line = ts.readLine();
        const QStringList &maps = line.split(' ', QString::SplitBehavior::SkipEmptyParts);
        if (Q_UNLIKELY(maps.size() < 6))
            continue;

        QFileInfo info(maps.value(5));
        const QString &infoAbPath = info.absoluteFilePath();
        if (modulePath == infoAbPath || !info.fileName().contains("dtkcore"))
            continue;

        if (modulePath.isEmpty()) {
            modulePath = infoAbPath;
        } else {
            // modulePath != infoAbPath
            QByteArray msg;
            msg += modulePath + " and " + info.absoluteFilePath() + " both loaded";
            qFatal("%s", msg.data());
        }
    }
}

// 在库被加载时就执行此函数
__attribute__((constructor)) void init()
{
    doubleLoadCheck();
}

int dtkVersion()
{
    return DTK_VERSION;
}

const char *dtkVersionString()
{
    return DTK_VERSION_STR;
}
