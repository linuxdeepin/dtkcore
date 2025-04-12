// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QCommandLineParser>
#include <QFileInfo>

static QString toUnicodeEscape(const QString& input) {
    QString result;
    for (QChar ch : input) {
        result += QString("\\u%1").arg(static_cast<short>(ch.unicode()), 4, 16, QChar('0'));
    }
    return result;
}

// Converts a QJsonValue to a corresponding C++ code representation
static QString jsonValueToCppCode(const QJsonValue &value){
    if (value.isBool()) {
        return value.toBool() ? QLatin1String("true") : QLatin1String("false");
    } else if (value.isDouble()) {
        const auto variantValue = value.toVariant();
        if (variantValue.userType() == QVariant(static_cast<int>(1)).userType()) {
            return QString::number(value.toInt());
        } else if (variantValue.userType() == QVariant(static_cast<qint64>(1)).userType()) {
            return QString::number(variantValue.toLongLong());
        }

        return QString::number(value.toDouble());
    } else if (value.isString()) {
        const auto string = value.toString();
        if (string.isEmpty()) {
            return QLatin1String("QLatin1String(\"\")");
        }
        return QString("QStringLiteral(u\"%1\")").arg(toUnicodeEscape(string));
    } else if (value.isNull()) {
        return "QVariant::fromValue(nullptr)";
    } else if (value.isArray()) {
        QStringList elements;
        const auto array = value.toArray();
        for (const QJsonValue &element : array) {
            elements << "QVariant(" + jsonValueToCppCode(element) + ")";
        }
        return "QList<QVariant>{" + elements.join(", ") + "}";
    } else if (value.isObject()) {
        QStringList elements;
        QJsonObject obj = value.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            elements << QString("{QStringLiteral(u\"%1\"), QVariant(%2)}")
                            .arg(toUnicodeEscape(it.key()),
                                 jsonValueToCppCode(it.value()));
        }
        return "QVariantMap{" + elements.join(", ") + "}";
    } else {
        return "QVariant()";
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(QLatin1String("DConfig to C++ class generator"));
    parser.addHelpOption();

    // Define command line options
    QCommandLineOption classNameOption(QStringList() << QLatin1String("c") << QLatin1String("class-name"),
                                       QLatin1String("Name of the generated class"),
                                       QLatin1String("className"));
    parser.addOption(classNameOption);

    QCommandLineOption sourceFileOption(QStringList() << QLatin1String("o") << QLatin1String("output"),
                                        QLatin1String("Path to the output source(header only) file"),
                                        QLatin1String("sourceFile"));
    parser.addOption(sourceFileOption);

    QCommandLineOption forceRequestThread(QStringList() << QLatin1String("force-request-thread"),
                                          QLatin1String("Force request thread to create DConfig instance"));
    parser.addOption(forceRequestThread);

    QCommandLineOption noComment(QStringList() << QLatin1String("no-comment"),
                                 QLatin1String("Do not generate comments in the generated code"));
    parser.addOption(noComment);

    parser.addPositionalArgument(QLatin1String("json-file"), QLatin1String("Path to the input JSON file"));
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        parser.showHelp(-1);
    }

    QString className = parser.value(classNameOption);
    const QString jsonFileName = QFileInfo(args.first()).completeBaseName();
    if (className.isEmpty()) {
        className = QLatin1String("dconfig_") + QString(jsonFileName).replace('.', '_');
    }

    QString sourceFilePath = parser.value(sourceFileOption);
    if (sourceFilePath.isEmpty()) {
        sourceFilePath = className.toLower() + QLatin1String(".hpp");
    }

    QFile file(args.first());
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << QLatin1String("Failed to open file:") << args.first();
        return -1;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    // Check magic value
    if (root[QLatin1String("magic")].toString() != QLatin1String("dsg.config.meta")) {
        qWarning() << QLatin1String("Invalid magic value in JSON file");
        return -1;
    }

    // Generate header and source files
    QFile headerFile(sourceFilePath);
    if (!headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << QLatin1String("Failed to open file for writing:") << sourceFilePath;
        return -1;
    }

    QTextStream headerStream(&headerFile);

    // Extract version and add it as a comment in the generated code
    QString version = root[QLatin1String("version")].toString();

    // Generate header and source file comments
    QString commandLineArgs = QCoreApplication::arguments().join(QLatin1String(" "));
    QString generationTime = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (!parser.isSet(noComment)) {
        QString headerComment = QString(
                                    "/**\n"
                                    " * This file is generated by dconfig2cpp.\n"
                                    " * Command line arguments: %1\n"
                                    " * Generation time: %2\n"
                                    " * JSON file version: %3\n"
                                    " *\n"
                                    " * WARNING: DO NOT MODIFY THIS FILE MANUALLY.\n"
                                    " * If you need to change the content, please modify the dconfig2cpp tool.\n"
                                    " */\n\n"
                                    ).arg(commandLineArgs, generationTime, version);

        headerStream << headerComment;
    }

    QJsonObject contents = root[QLatin1String("contents")].toObject();

    // Write header file content
    headerStream << "#ifndef " << className.toUpper() << "_H\n";
    headerStream << "#define " << className.toUpper() << "_H\n\n";
    headerStream << "#include <QThread>\n";
    headerStream << "#include <QVariant>\n";
    headerStream << "#include <QDebug>\n";
    headerStream << "#include <QAtomicPointer>\n";
    headerStream << "#include <QAtomicInteger>\n";
    headerStream << "#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)\n"
                 << "#include <QProperty>\n"
                 << "#endif\n";

    headerStream << "#include <DSGApplication>\n";
    headerStream << "#include <DConfig>\n\n";
    headerStream << "class " << className << " : public QObject {\n";
    headerStream << "    Q_OBJECT\n\n";

    struct Property {
        QString typeName;
        QString propertyName;
        QString capitalizedPropertyName;
        QString propertyNameString;
        QJsonValue defaultValue;
    };
    QList<Property> properties;
    QStringList propertyNames;
    QStringList propertyNameStrings;

    static QStringList usedKeywords = {
        className,
        "create",
        "createByName",
        "config",
        "keyList",
        "isInitializeSucceed",
        "isInitializeFailed",
        "isInitializing",
        "isDefaultValue",
        "m_config",
        "m_status",
    };

    for (int i = 0; i <= (contents.size()) / 32; ++i) {
        usedKeywords << QLatin1String("m_propertySetStatus") + QString::number(i);
    }

    // Iterate over JSON contents to extract properties
    for (auto it = contents.begin(); it != contents.end(); ++it) {
        QJsonObject obj = it.value().toObject();
        QString propertyName = it.key();
        QString typeName;
        const auto value = obj[QLatin1String("value")];
        if (value.isBool()) {
            typeName = "bool";
        } else if (value.isArray()) {
            typeName = "QList<QVariant>";
        } else if (value.isObject()) {
            typeName = "QVariantMap";
        } else if (value.isDouble()) {
            const auto variantValue = value.toVariant();
            typeName = QString::fromLatin1(variantValue.typeName());
        } else if (value.isString()) {
            typeName = "QString";
        } else {
            typeName = "QVariant";
        }

        QString capitalizedPropertyName = propertyName;
        if (!capitalizedPropertyName.isEmpty() && capitalizedPropertyName[0].isLower()) {
            capitalizedPropertyName[0] = capitalizedPropertyName[0].toUpper();
        }

        propertyNames << propertyName;
        properties.append(Property({
            typeName,
            propertyName,
            capitalizedPropertyName,
            "QStringLiteral(\"" + propertyName + "\")",
            obj[QLatin1String("value")]
        }));
        propertyNameStrings << properties.last().propertyNameString;

        const  QString readFunction = usedKeywords.contains(propertyName) ? QLatin1String(" READ get") + capitalizedPropertyName
                                                                           : QLatin1String(" READ ") + propertyName;
        headerStream << "    Q_PROPERTY(" << typeName << " " << propertyName << readFunction
                     << " WRITE set" << capitalizedPropertyName << " NOTIFY " << propertyName << "Changed"
                     << " RESET reset" << capitalizedPropertyName << ")\n";
    }

    headerStream << "    Q_CLASSINFO(\"DConfigKeyList\", \"" << propertyNames.join(";") <<"\")\n"
                 << "    Q_CLASSINFO(\"DConfigFileName\", \"" << QString(jsonFileName).replace("\n", "\\n").replace("\r", "\\r") <<"\")\n"
                 << "    Q_CLASSINFO(\"DConfigFileVersion\", \"" << version <<"\")\n\n"
                 << "public:\n"
                 << "    explicit " << className
                 << R"((QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend,
                        const QString &name, const QString &appId, const QString &subpath,
                        bool isGeneric, QObject *parent)
                : QObject(parent) {
        if (!thread->isRunning()) {
            qWarning() << QLatin1String("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=, this]() {
            DTK_CORE_NAMESPACE::DConfig *config = nullptr;
            if (isGeneric) {
                if (backend) {
                    config = DTK_CORE_NAMESPACE::DConfig::createGeneric(backend, name, subpath, nullptr);
                } else {
                    config = DTK_CORE_NAMESPACE::DConfig::createGeneric(name, subpath, nullptr);
                }
            } else {
                if (backend) {
                    if (appId.isNull()) {
                        config = DTK_CORE_NAMESPACE::DConfig::create(backend, DTK_CORE_NAMESPACE::DSGApplication::id(),
                                                                     name, subpath, nullptr);
                    } else {
                        config = DTK_CORE_NAMESPACE::DConfig::create(backend, appId, name, subpath, nullptr);
                    }
                } else {
                    if (appId.isNull()) {
                        config = DTK_CORE_NAMESPACE::DConfig::create(DTK_CORE_NAMESPACE::DSGApplication::id(),
                                                                     name, subpath, nullptr);
                    } else {
                        config = DTK_CORE_NAMESPACE::DConfig::create(appId, name, subpath, nullptr);
                    }
                }
            }
            if (!config) {
                qWarning() << QLatin1String("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initializeInConfigThread(config);
            worker->deleteLater();
        });
    }
)";

    const QString jsonFileString = "QStringLiteral(u\"" + toUnicodeEscape(jsonFileName) + "\")";
    // Generate constructors
    if (parser.isSet(forceRequestThread))
        headerStream << "    static " << className << "* create(QThread *thread, const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr)\n";
    else
        headerStream << "    static " << className << "* create(const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
    headerStream << "    { return new " << className << "(thread, nullptr, " << jsonFileString << ", appId, subpath, false, parent); }\n";
    if (parser.isSet(forceRequestThread))
        headerStream << "    static " << className << "* create(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr)\n";
    else
        headerStream << "    static " << className << "* create(DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
    headerStream << "    { return new " << className << "(thread, backend, " << jsonFileString << ", appId, subpath, false, parent); }\n";
    if (parser.isSet(forceRequestThread))
        headerStream << "    static " << className << "* createByName(QThread *thread, const QString &name, const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr)\n";
    else
        headerStream << "    static " << className << "* createByName(const QString &name, const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
    headerStream << "    { return new " << className << "(thread, nullptr, name, appId, subpath, false, parent); }\n";
    if (parser.isSet(forceRequestThread))
        headerStream << "    static " << className << "* createByName(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr)\n";
    else
        headerStream << "    static " << className << "* createByName(DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
    headerStream << "    { return new " << className << "(thread, backend, name, appId, subpath, false, parent); }\n";

    if (parser.isSet(forceRequestThread))
        headerStream << "    static " << className << "* createGeneric(QThread *thread, const QString &subpath = {}, QObject *parent = nullptr)\n";
    else
        headerStream << "    static " << className << "* createGeneric(const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
    headerStream << "    { return new " << className << "(thread, nullptr, " << jsonFileString << ", {}, subpath, true, parent); }\n";
    if (parser.isSet(forceRequestThread))
        headerStream << "    static " << className << "* create(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &subpath = {}, QObject *parent = nullptr)\n";
    else
        headerStream << "    static " << className << "* create(DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
    headerStream << "    { return new " << className << "(thread, backend, " << jsonFileString << ", {}, subpath, true, parent); }\n";
    if (parser.isSet(forceRequestThread))
        headerStream << "    static " << className << "* createGenericByName(QThread *thread, const QString &name, const QString &subpath = {}, QObject *parent = nullptr)\n";
    else
        headerStream << "    static " << className << "* createGenericByName(const QString &name, const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
    headerStream << "    { return new " << className << "(thread, nullptr, name, {}, subpath, true, parent); }\n";
    if (parser.isSet(forceRequestThread))
        headerStream << "    static " << className << "* createGenericByName(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath = {}, QObject *parent = nullptr)\n";
    else
        headerStream << "    static " << className << "* createGenericByName(DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
    headerStream << "    { return new " << className << "(thread, backend, name, {}, subpath, true, parent); }\n";

    // Destructor
    headerStream << "    ~" << className << R"(() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    Q_INVOKABLE DTK_CORE_NAMESPACE::DConfig *config() const {
        return m_config.loadRelaxed();
    }

    Q_INVOKABLE bool isInitializeSucceed() const {
        return m_status.loadRelaxed() == static_cast<int>(Status::Succeed);
    }

    Q_INVOKABLE bool isInitializeFailed() const {
        return m_status.loadRelaxed() == static_cast<int>(Status::Failed);
    }

    Q_INVOKABLE bool isInitializing() const {
        return m_status.loadRelaxed() == static_cast<int>(Status::Invalid);
    }

)";

    headerStream << "    Q_INVOKABLE QStringList keyList() const {\n"
                 << "        return { " << propertyNameStrings.join(",\n                 ") << "};\n"
                 << "    }\n\n";

    headerStream << "    Q_INVOKABLE bool isDefaultValue(const QString &key) const {\n";
    for (int i = 0; i < properties.size(); ++i) {
        headerStream << "        if (key == " << properties.at(i).propertyNameString << ")\n"
                     << "            return " << properties.at(i).propertyName << "IsDefaultValue();\n";
    }
    headerStream << "        return false;\n"
                 << "    }\n\n";

    // Generate property getter and setter methods
    for (int i = 0; i < properties.size(); ++i) {
        const Property &property = properties[i];
        const  QString readFunction = usedKeywords.contains(property.propertyName)
                                         ? "get" + property.capitalizedPropertyName
                                         : property.propertyName;
        assert(!usedKeywords.contains(readFunction));

        headerStream << "    " << property.typeName << " " << readFunction << "() const {\n"
                     << "        return p_" << property.propertyName << ";\n    }\n";
        headerStream << "    void set" << property.capitalizedPropertyName << "(const " << property.typeName << " &value) {\n"
                     << "        auto oldValue = p_" << property.propertyName << ";\n"
                     << "        p_" << property.propertyName << " = value;\n"
                     << "        markPropertySet(" << i << ");\n"
                     << "        if (auto config = m_config.loadRelaxed()) {\n"
                     << "            QMetaObject::invokeMethod(config, [this, value]() {\n"
                     << "                m_config.loadRelaxed()->setValue(" << property.propertyNameString << ", value);\n"
                     << "            });\n"
                     << "        }\n"
                     << "        if (p_" << property.propertyName << " != oldValue) {\n"
                     << "            Q_EMIT " << property.propertyName << "Changed();\n"
                     << "            Q_EMIT valueChanged(" << property.propertyNameString << ", value);\n"
                     << "        }\n"
                     << "    }\n"
                     << "    void reset" << property.capitalizedPropertyName << "() {\n"
                     << "        if (auto config = m_config.loadRelaxed()) {\n"
                     << "            QMetaObject::invokeMethod(config, [this]() {\n"
                     << "                m_config.loadRelaxed()->reset(" << property.propertyNameString << ");\n"
                     << "            });\n"
                     << "        }\n"
                     << "    }\n";
        headerStream << "#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)\n";
        headerStream << "    QBindable<" << property.typeName << "> bindable" << property.capitalizedPropertyName << "() {\n"
                     << "        return QBindable<" << property.typeName << ">(this, \"" << property.propertyName << "\");\n"
                     << "    }\n";
        headerStream << "#endif\n";

        headerStream << "    Q_INVOKABLE bool " << property.propertyName << "IsDefaultValue() const {\n"
                     << "        return !testPropertySet(" << i << ");\n"
                     << "    }\n";
    }

    // Generate signals for property changes
    headerStream << "Q_SIGNALS:\n"
                 << "    void configInitializeFailed(DTK_CORE_NAMESPACE::DConfig *config);\n"
                 << "    void configInitializeSucceed(DTK_CORE_NAMESPACE::DConfig *config);\n"
                 << "    void valueChanged(const QString &key, const QVariant &value);\n\n";
    for (const Property &property : std::as_const(properties)) {
        headerStream << "    void " << property.propertyName << "Changed();\n";
    }

    // Generate private methods and members
    headerStream << "private:\n";

    headerStream << "    void initializeInConfigThread(DTK_CORE_NAMESPACE::DConfig *config) {\n"
                 << "        Q_ASSERT(!m_config.loadRelaxed());\n"
                 << "        m_config.storeRelaxed(config);\n"
                 << "        if (!config->isValid()) {\n"
                 << "           m_status.storeRelaxed(static_cast<int>(Status::Failed));\n"
                 << "           Q_EMIT configInitializeFailed(config);\n"
                 << "           return;\n"
                 << "        }\n\n";
    for (int i = 0; i < properties.size(); ++i) {
        const Property &property = properties[i];
        headerStream << "        if (testPropertySet(" << i << ")) {\n";
        headerStream << "            config->setValue(" << property.propertyNameString << ", QVariant::fromValue(p_" << property.propertyName << "));\n";
        headerStream << "        } else {\n";
        headerStream << "            updateValue(" << property.propertyNameString << ", QVariant::fromValue(p_" << property.propertyName << "));\n";
        headerStream << "        }\n";
    }
    headerStream << R"(
        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);

        m_status.storeRelaxed(static_cast<int>(Status::Succeed));
        Q_EMIT configInitializeSucceed(config);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
)";
    for (int i = 0; i < properties.size(); ++i) {
        const Property &property = properties.at(i);
        headerStream << "        if (key == " << property.propertyNameString << ") {\n";

        headerStream << "            markPropertySet(" << i << ", !m_config.loadRelaxed()->isDefaultValue(key));\n";

        headerStream << "            auto newValue = qvariant_cast<" << property.typeName << ">(value);\n"
                     << "            QMetaObject::invokeMethod(this, [this, newValue, key, value]() {\n"
                     << "                Q_ASSERT(QThread::currentThread() == this->thread());\n"
                     << "                if (p_" << property.propertyName << " != newValue) {\n"
                     << "                    p_" << property.propertyName << " = newValue;\n"
                     << "                    Q_EMIT " << property.propertyName << "Changed();\n"
                     << "                    Q_EMIT valueChanged(key, value);\n"
                     << "                }\n"
                     << "            });\n"
                     << "            return;\n"
                     << "        }\n";
    }
    headerStream << "    }\n";

    // Mark property as set
    headerStream << "    inline void markPropertySet(const int index, bool on = true) {\n";
    for (int i = 0; i <= (properties.size()) / 32; ++i) {
        headerStream << "        if (index < " << (i + 1) * 32 << ") {\n"
                     << "            if (on)\n"
                     << "                m_propertySetStatus" << QString::number(i) << ".fetchAndOrOrdered(1 << (index - " << i * 32 << "));\n"
                     << "            else\n"
                     << "                m_propertySetStatus" << QString::number(i) << ".fetchAndAndOrdered(1 << (index - " << i * 32 << "));\n"
                     << "            return;\n"
                     << "        }\n";
    }
    headerStream << "        Q_UNREACHABLE();\n    }\n";

    // Test if property is set
    headerStream << "    inline bool testPropertySet(const int index) const {\n";
    for (int i = 0; i <= (properties.size()) / 32; ++i) {
        headerStream << "        if (index < " << (i + 1) * 32 << ") {\n";
        headerStream << "            return (m_propertySetStatus" << QString::number(i) << ".loadRelaxed() & (1 << (index - " << i * 32 << ")));\n";
        headerStream << "        }\n";
    }
    headerStream << "        Q_UNREACHABLE();\n"
                 << "    }\n";

    // Member variables
    headerStream << R"(
    QAtomicPointer<DTK_CORE_NAMESPACE::DConfig> m_config = nullptr;

public:
    enum class Status {
        Invalid = 0,
        Succeed = 1,
        Failed = 2
    };
private:
    QAtomicInteger<int> m_status = static_cast<int>(Status::Invalid);

)";

    // Property variables
    for (const Property &property : std::as_const(properties)) {
        if (property.typeName == QLatin1String("int") || property.typeName == QLatin1String("qint64")) {
            headerStream << "    // Note: If you expect a double type, add 'e' to the number in the JSON value field, e.g., \"value\": 1.0e, not just 1.0\n";
        } else if (property.typeName == QLatin1String("QString")) {
            headerStream << "    // Default value: \"" << property.defaultValue.toString().replace("\n", "\\n").replace("\r", "\\r") << "\"\n";
        }
        headerStream << "    " << property.typeName << " p_" << property.propertyName << " { ";
        headerStream << jsonValueToCppCode(property.defaultValue) << " };\n";
    }

    // Property set status variables
    for (int i = 0; i <= (properties.size()) / 32; ++i) {
        headerStream << "    QAtomicInteger<quint32> m_propertySetStatus" << QString::number(i) << " = 0;\n";
    }
    headerStream << "};\n\n";
    headerStream << "#endif // " << className.toUpper() << "_H\n";

    return 0;
}
