#include "gsettingsbackend.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <QGSettings/QGSettings>

#include <DSettings>

DCORE_BEGIN_NAMESPACE


QString unqtifyName(const QString &name)
{
    QString ret;
    for (auto p : name) {
        const QChar c(p);
        if (c.isUpper()) {
            ret.append("-");
            ret.append(c.toLower().toLatin1());
        } else {
            ret.append(c);
        }
    }
    return ret;
}

QString qtifyName(const QString &key)
{
    return QString(key).replace(".", "-").replace("_", "-");
}


class GSettingsBackendPrivate
{
public:
    GSettingsBackendPrivate(GSettingsBackend *parent) : q_ptr(parent) {}

    QGSettings *settings;
    QMap<QString, QString> keyMap;

    GSettingsBackend *q_ptr;
    Q_DECLARE_PUBLIC(GSettingsBackend)
};

GSettingsBackend::GSettingsBackend(DSettings *settings, QObject *parent) :
    DSettingsBackend(parent), d_ptr(new GSettingsBackendPrivate(this))
{
    Q_D(GSettingsBackend);

    QJsonObject settingsMeta = settings->meta();
    auto gsettingsMeta = settingsMeta.value("gsettings").toObject();
    auto id = gsettingsMeta.value("id").toString();
    auto path = gsettingsMeta.value("path").toString();

    for (QString key : settings->keys()) {
        auto gsettingsKey = QString(key).replace(".", "-").replace("_", "-");
        d->keyMap.insert(gsettingsKey, key);
    }

    d->settings = new QGSettings(id.toUtf8(), path.toUtf8(), this);

    connect(d->settings, &QGSettings::changed, this, [ = ](const QString & key) {
        auto dk = d->keyMap.value(unqtifyName(key));
        Q_EMIT optionChanged(dk, d->settings->get(key));
    });

}

GSettingsBackend::~GSettingsBackend()
{

}

QStringList GSettingsBackend::keys() const
{
    Q_D(const GSettingsBackend);
    return d->settings->keys();
}

QVariant GSettingsBackend::getOption(const QString &key) const
{
    Q_D(const GSettingsBackend);
    return d->settings->get(qtifyName(key));
}

void GSettingsBackend::doSetOption(const QString &key, const QVariant &value)
{
    Q_D(GSettingsBackend);
    d->settings->set(qtifyName(key), value);
    Q_EMIT setOption(key, value);
}

void GSettingsBackend::doSync()
{
    Q_EMIT sync();
}

DCORE_END_NAMESPACE
