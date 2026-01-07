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
#include <QDateTime>

static QString toUnicodeEscape(const QString& input) {
    QString result;
    for (QChar ch : input) {
        result += QString("\\u%1").arg(static_cast<short>(ch.unicode()), 4, 16, QChar('0'));
    }
    return result;
}

// Check if the original JSON value is in floating-point format
static bool isOriginalValueFloat(const QByteArray& jsonData, const QString& propertyName, const QJsonValue& value) {
    if (!value.isDouble()) {
        return false;
    }
    // Build search pattern to find the number in "propertyName": { "value": number }
    QString searchPattern = QString("\"%1\"\\s*:\\s*\\{[^}]*\"value\"\\s*:\\s*([0-9.eE+-]+)").arg(propertyName);
    QRegularExpression regex(searchPattern);
    QString jsonString = QString::fromUtf8(jsonData);
    QRegularExpressionMatch match = regex.match(jsonString);
    if (match.hasMatch()) {
        QString numberStr = match.captured(1);
        // If contains decimal point or scientific notation, it's a floating-point number
        return numberStr.contains('.') || numberStr.contains('e', Qt::CaseInsensitive);
    }
    return false;
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
                            .arg(toUnicodeEscape(it.key()), jsonValueToCppCode(it.value()));
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
        QString headerComment = QString("/**\n"
                                        " * This file is generated by dconfig2cpp.\n"
                                        " * Command line arguments: %1\n"
                                        " * Generation time: %2\n"
                                        " * JSON file version: %3\n"
                                        " *\n"
                                        " * WARNING: DO NOT MODIFY THIS FILE MANUALLY.\n"
                                        " * If you need to change the content, please modify the dconfig2cpp tool.\n"
                                        " */\n\n")
                                    .arg(commandLineArgs, generationTime, version);
        headerStream << headerComment;
    }

    QJsonObject contents = root[QLatin1String("contents")].toObject();

    // Write header file content
    headerStream << "#ifndef " << className.toUpper() << "_H\n";
    headerStream << "#define " << className.toUpper() << "_H\n\n";
    headerStream << "#include <QThread>\n";
    headerStream << "#include <QVariant>\n";
    headerStream << "#include <QPointer>\n";
    headerStream << "#include <QDebug>\n";
    headerStream << "#include <QAtomicPointer>\n";
    headerStream << "#include <QAtomicInteger>\n";
    headerStream << "#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)\n"
                 << "#include <QProperty>\n"
                 << "#endif\n";

    headerStream << "#include <DSGApplication>\n";
    headerStream << "#include <DConfig>\n\n";

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
        "m_data",
    };

    for (int i = 0; i <= (contents.size()) / 32; ++i) {
        usedKeywords << QLatin1String("m_propertySetStatus") + QString::number(i);
    }

    // Extract properties from JSON
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
            // QJson treats all numbers in JSON as double. Although converting QJsonValue to
            // QVariant can distinguish between double and int, Qt's JSON parsing attempts to
            // convert floating-point numbers with decimal parts of 0, such as 1.0, to integers,
            // resulting in the generated property type being qlonglong. However, dconfig expects
            // floating-point numbers, so we try to identify whether it is a floating-point number
            // or an integer by matching strings.
            if (isOriginalValueFloat(data, propertyName, value)) {
                typeName = "double";
            } else {
                typeName = "qlonglong";
            }
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
        properties.append(Property({typeName,
                                    propertyName,
                                    capitalizedPropertyName,
                                    "QStringLiteral(\"" + propertyName + "\")",
                                    obj[QLatin1String("value")]}));
        propertyNameStrings << properties.last().propertyNameString;
    }

    // ==================== Forward Declarations ====================
    headerStream << "class " << className << ";\n\n";

    // ==================== TreelandUserConfigData definition ====================
    headerStream << "class " << className << "Data : public QObject {\n"
                 << "public:\n"
                 << "    enum class Status {\n"
                 << "        Invalid = 0,\n"
                 << "        Initializing = 1,\n"
                 << "        Succeed = 2,\n"
                 << "        Failed = 3,\n"
                 << "        Destroyed = 4\n"
                 << "    };\n"
                 << "\n"
                 << "    explicit " << className << "Data(QObject *parent = nullptr)\n"
                 << "        : QObject(parent) {}\n"
                 << "\n"
                 << "    void initializeInConfigThread(DTK_CORE_NAMESPACE::DConfig *config);\n"
                 << "    void updateValue(const QString &key, const QVariant &fallback = QVariant());\n"
                 << "    void updateProperty(const QString &key, const QVariant &value);\n"
                 << "\n"
                 << "    inline void markPropertySet(const int index, bool on = true) {\n";

    for (int i = 0; i <= (properties.size()) / 32; ++i) {
        headerStream << "        if (index < " << (i + 1) * 32 << ") {\n"
                     << "            if (on)\n"
                     << "                m_propertySetStatus" << i << ".fetchAndOrOrdered(1 << (index - " << i * 32 << "));\n"
                     << "            else\n"
                     << "                m_propertySetStatus" << i << ".fetchAndAndOrdered(~(1 << (index - " << i * 32 << ")));\n"
                     << "            return;\n"
                     << "        }\n";
    }
    headerStream << "        Q_UNREACHABLE();\n"
                 << "    }\n"
                 << "\n"
                 << "    inline bool testPropertySet(const int index) const {\n";

    for (int i = 0; i <= (properties.size()) / 32; ++i) {
        headerStream << "        if (index < " << (i + 1) * 32 << ") {\n"
                     << "            return (m_propertySetStatus" << i << ".loadRelaxed() & (1 << (index - " << i * 32 << ")));\n"
                     << "        }\n";
    }
    headerStream << "        Q_UNREACHABLE();\n"
                 << "    }\n\n"
                 << "    QAtomicPointer<DTK_CORE_NAMESPACE::DConfig> m_config = nullptr;\n"
                 << "    QAtomicInteger<int> m_status = static_cast<int>(Status::Invalid);\n"
                 << "    QPointer<" << className << "> m_userConfig = nullptr;\n";

    for (int i = 0; i <= (properties.size()) / 32; ++i) {
        headerStream << "    QAtomicInteger<quint32> m_propertySetStatus" << i << " = 0;\n";
    }

    headerStream << "\n    // Property storage\n";
    for (const Property &property : properties) {
        if (property.typeName == QLatin1String("QString")) {
            headerStream << "    // Default value: \""
                         << property.defaultValue.toString().replace("\n", "\\n").replace("\r", "\\r") << "\"\n";
        }
        headerStream << "    " << property.typeName << " p_" << property.propertyName << " { "
                     << jsonValueToCppCode(property.defaultValue) << " };\n";
    }
    headerStream << "};\n\n";

    // ==================== TreelandUserConfig definition ====================
    headerStream << "class " << className << " : public QObject {\n"
                 << "    Q_OBJECT\n"
                 << "public:\n"
                 << "    using Data = " << className << "Data;\n\n";

    // Generate Q_PROPERTY declarations
    for (const Property &property : properties) {
        const QString readFunction = usedKeywords.contains(property.propertyName) ?
                                         QLatin1String(" READ get") + property.capitalizedPropertyName :
                                         QLatin1String(" READ ") + property.propertyName;
        headerStream << "    Q_PROPERTY(" << property.typeName << " " << property.propertyName << readFunction << " WRITE set"
                     << property.capitalizedPropertyName << " NOTIFY " << property.propertyName << "Changed"
                     << " RESET reset" << property.capitalizedPropertyName << ")\n";
    }

    headerStream << "    Q_CLASSINFO(\"DConfigKeyList\", \"" << propertyNames.join(";") << "\")\n"
                 << "    Q_CLASSINFO(\"DConfigFileName\", \"" << QString(jsonFileName).replace("\n", "\\n").replace("\r", "\\r")
                 << "\")\n"
                 << "    Q_CLASSINFO(\"DConfigFileVersion\", \"" << version << "\")\n\n"
                 << "public:\n"
                 << "    explicit " << className << R"((QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend,
                        const QString &name, const QString &appId, const QString &subpath,
                        bool isGeneric, QObject *parent)
                  : QObject(parent), m_data(new Data) {
        m_data->m_userConfig = this;

        if (!thread->isRunning()) {
            qWarning() << QLatin1String("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        auto data = m_data;

        QMetaObject::invokeMethod(worker, [=]() {
            // Atomically transition from Invalid to Initializing
            if (!data->m_status.testAndSetOrdered(static_cast<int>(Data::Status::Invalid),
                                                  static_cast<int>(Data::Status::Initializing))) {
                worker->deleteLater();
                return;
            }

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

            if (!config || !config->isValid()) {
                qWarning() << QLatin1String("Failed to create DConfig instance.");

                if (data->m_status.testAndSetOrdered(static_cast<int>(Data::Status::Initializing),
                                                     static_cast<int>(Data::Status::Failed))) {
                     // Successfully transitioned to Failed - notify main thread
                    QMetaObject::invokeMethod(data, [data]() {
                        if (data->m_userConfig)
                            Q_EMIT data->m_userConfig->configInitializeFailed();
                    });
                }

                worker->deleteLater();
                if (config)
                    delete config;

                return;
            }

            config->moveToThread(QThread::currentThread());

            // Initialize through Data class
            data->initializeInConfigThread(config);

            QObject::connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, data, [data](const QString &key) { data->updateValue(key); }, Qt::DirectConnection);
            QObject::connect(config, &QObject::destroyed, data, &QObject::deleteLater);
            QMetaObject::invokeMethod(data, [data, config]() {
                if (data->m_userConfig)
                    Q_EMIT data->m_userConfig->configInitializeSucceed(config);
            });
            worker->deleteLater();
        });
    }
)";

    const QString jsonFileString = "QStringLiteral(u\"" + toUnicodeEscape(jsonFileName) + "\")";

    // Create factory methods
    if (parser.isSet(forceRequestThread)) {
        headerStream
            << "    static " << className
            << "* create(QThread *thread, const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr)\n";
        headerStream << "    { return new " << className << "(thread, nullptr, " << jsonFileString
                     << ", appId, subpath, false, parent); }\n";
        headerStream << "    static " << className
                     << "* create(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId = {}, const "
                        "QString &subpath = {}, QObject *parent = nullptr)\n";
        headerStream << "    { return new " << className << "(thread, backend, " << jsonFileString
                     << ", appId, subpath, false, parent); }\n";
        headerStream << "    static " << className
                     << "* createByName(QThread *thread, const QString &name, const QString &appId = {}, const QString &subpath "
                        "= {}, QObject *parent = nullptr)\n";
        headerStream << "    { return new " << className << "(thread, nullptr, name, appId, subpath, false, parent); }\n";
        headerStream << "    static " << className
                     << "* createByName(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const "
                        "QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr)\n";
        headerStream << "    { return new " << className << "(thread, backend, name, appId, subpath, false, parent); }\n";
        headerStream << "    static " << className
                     << "* createGeneric(QThread *thread, const QString &subpath = {}, QObject *parent = nullptr)\n";
        headerStream << "    { return new " << className << "(thread, nullptr, " << jsonFileString
                     << ", {}, subpath, true, parent); }\n";
        headerStream << "    static " << className
                     << "* create(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &subpath = {}, "
                        "QObject *parent = nullptr)\n";
        headerStream << "    { return new " << className << "(thread, backend, " << jsonFileString
                     << ", {}, subpath, true, parent); }\n";
        headerStream << "    static " << className
                     << "* createGenericByName(QThread *thread, const QString &name, const QString &subpath = {}, QObject "
                        "*parent = nullptr)\n";
        headerStream << "    { return new " << className << "(thread, nullptr, name, {}, subpath, true, parent); }\n";
        headerStream << "    static " << className
                     << "* createGenericByName(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString "
                        "&name, const QString &subpath = {}, QObject *parent = nullptr)\n";
        headerStream << "    { return new " << className << "(thread, backend, name, {}, subpath, true, parent); }\n";
    } else {
        headerStream << "    static " << className
                     << "* create(const QString &appId = {}, const QString &subpath = {}, QObject *parent = nullptr, QThread "
                        "*thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
        headerStream << "    { return new " << className << "(thread, nullptr, " << jsonFileString
                     << ", appId, subpath, false, parent); }\n";
        headerStream << "    static " << className
                     << "* create(DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId = {}, const QString &subpath "
                        "= {}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
        headerStream << "    { return new " << className << "(thread, backend, " << jsonFileString
                     << ", appId, subpath, false, parent); }\n";
        headerStream << "    static " << className
                     << "* createByName(const QString &name, const QString &appId = {}, const QString &subpath = {}, QObject "
                        "*parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
        headerStream << "    { return new " << className << "(thread, nullptr, name, appId, subpath, false, parent); }\n";
        headerStream << "    static " << className
                     << "* createByName(DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &appId = "
                        "{}, const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = "
                        "DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
        headerStream << "    { return new " << className << "(thread, backend, name, appId, subpath, false, parent); }\n";
        headerStream << "    static " << className
                     << "* createGeneric(const QString &subpath = {}, QObject *parent = nullptr, QThread *thread = "
                        "DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
        headerStream << "    { return new " << className << "(thread, nullptr, " << jsonFileString
                     << ", {}, subpath, true, parent); }\n";
        headerStream << "    static " << className
                     << "* create(DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &subpath = {}, QObject *parent = "
                        "nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
        headerStream << "    { return new " << className << "(thread, backend, " << jsonFileString
                     << ", {}, subpath, true, parent); }\n";
        headerStream << "    static " << className
                     << "* createGenericByName(const QString &name, const QString &subpath = {}, QObject *parent = nullptr, "
                        "QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
        headerStream << "    { return new " << className << "(thread, nullptr, name, {}, subpath, true, parent); }\n";
        headerStream
            << "    static " << className
            << "* createGenericByName(DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath = "
               "{}, QObject *parent = nullptr, QThread *thread = DTK_CORE_NAMESPACE::DConfig::globalThread())\n";
        headerStream << "    { return new " << className << "(thread, backend, name, {}, subpath, true, parent); }\n";
    }

    // Destructor
    headerStream << "\n    ~" << className << R"(() {
        int oldStatus = m_data->m_status.fetchAndStoreOrdered(static_cast<int>(Data::Status::Destroyed));
        m_data->m_userConfig = nullptr;

        if (oldStatus == static_cast<int>(Data::Status::Succeed)) {
            if (auto config = m_data->m_config.loadRelaxed()) {
                config->deleteLater();
                // m_data will be deleted by config->destroyed connection
            } else {
                m_data->deleteLater();
            }
        } else if (oldStatus == static_cast<int>(Data::Status::Failed) || oldStatus == static_cast<int>(Data::Status::Invalid)) {
            m_data->deleteLater();
        }
        // If Initializing, worker thread handles m_data deletion when it sees Destroyed status
    }

    Q_INVOKABLE DTK_CORE_NAMESPACE::DConfig *config() const { return m_data->m_config.loadRelaxed(); }
    Q_INVOKABLE bool isInitializeSucceed() const { return m_data->m_status.loadRelaxed() == static_cast<int>(Data::Status::Succeed); }
    Q_INVOKABLE bool isInitializeFailed() const { return m_data->m_status.loadRelaxed() == static_cast<int>(Data::Status::Failed); }
    Q_INVOKABLE bool isInitializing() const { return m_data->m_status.loadRelaxed() == static_cast<int>(Data::Status::Initializing); }

    Q_INVOKABLE QStringList keyList() const { return { )"
                 << propertyNameStrings.join(", ") << R"( }; }

    Q_INVOKABLE bool isDefaultValue(const QString &key) const {
)";
    for (const auto &p : properties) {
        headerStream << "        if (key == " << p.propertyNameString << ") return " << p.propertyName << "IsDefaultValue();\n";
    }
    headerStream << "        return false;\n    }\n\n";

    // Property getters/setters/signals
    for (int i = 0; i < properties.size(); ++i) {
        const auto &p = properties[i];
        const QString read = usedKeywords.contains(p.propertyName) ? "get" + p.capitalizedPropertyName : p.propertyName;
        headerStream << "    " << p.typeName << " " << read << "() const { return m_data->p_" << p.propertyName << "; }\n"
                     << "    void set" << p.capitalizedPropertyName << "(const " << p.typeName << " &v) {\n"
                     << "        if (m_data->p_" << p.propertyName << " == v && m_data->testPropertySet(" << i << ")) return;\n"
                     << "        m_data->p_" << p.propertyName << " = v;\n"
                     << "        m_data->markPropertySet(" << i << ");\n"
                     << "        if (auto config = m_data->m_config.loadRelaxed()) {\n"
                     << "            QMetaObject::invokeMethod(config, [config, v]() { config->setValue(" << p.propertyNameString
                     << ", v); });\n"
                     << "        }\n"
                     << "        Q_EMIT " << p.propertyName << "Changed();\n"
                     << "        Q_EMIT valueChanged(" << p.propertyNameString << ", v);\n"
                     << "    }\n"
                     << "    void reset" << p.capitalizedPropertyName << "() {\n"
                     << "        if (auto config = m_data->m_config.loadRelaxed()) {\n"
                     << "            QMetaObject::invokeMethod(config, [config]() { config->reset(" << p.propertyNameString
                     << "); });\n"
                     << "        }\n"
                     << "    }\n"
                     << "    Q_INVOKABLE bool " << p.propertyName << "IsDefaultValue() const { return !m_data->testPropertySet("
                     << i << "); }\n";
    }

    headerStream << "\nQ_SIGNALS:\n"
                 << "    void configInitializeFailed();\n"
                 << "    void configInitializeSucceed(DTK_CORE_NAMESPACE::DConfig *config);\n"
                 << "    void valueChanged(const QString &key, const QVariant &value);\n";
    for (const auto &p : properties)
        headerStream << "    void " << p.propertyName << "Changed();\n";

    headerStream << "\nprivate:\n    Data *m_data;\n};\n\n";

    // ==================== TreelandUserConfigData Implementation ====================
    headerStream
        << "inline void " << className << "Data::initializeInConfigThread(DTK_CORE_NAMESPACE::DConfig *config) {\n"
        << "    Q_ASSERT(!m_config.loadRelaxed());\n"
        << "    m_config.storeRelaxed(config);\n";

    for (int i = 0; i < properties.size(); ++i) {
        const auto &p = properties[i];
        headerStream << "    if (testPropertySet(" << i << ")) config->setValue(" << p.propertyNameString
                     << ", QVariant::fromValue(p_" << p.propertyName << "));\n"
                     << "    else updateValue(" << p.propertyNameString << ", QVariant::fromValue(p_" << p.propertyName
                     << "));\n";
    }

    headerStream << R"(    // Transition from Initializing to Succeed
    if (!m_status.testAndSetOrdered(static_cast<int>(Status::Initializing), static_cast<int>(Status::Succeed))) {
        if (m_status.loadRelaxed() == static_cast<int>(Status::Destroyed)) {
            config->deleteLater();
            deleteLater();
        }
        return;
    }}

inline void )" << className
                 << R"(Data::updateValue(const QString &key, const QVariant &fallback) {
    auto config = m_config.loadRelaxed();
    if (!config) return;
    const QVariant &v = config->value(key, fallback);
)";
    for (int i = 0; i < properties.size(); ++i) {
        const auto &p = properties[i];
        headerStream << "    if (key == " << p.propertyNameString << ") {\n"
                     << "        markPropertySet(" << i << ", !config->isDefaultValue(key));\n"
                     << "        // Safe capture using QPointer to handle object destruction\n"
                     << "        QPointer<" << className << "Data> safeThis(this);\n"
                     << "        QMetaObject::invokeMethod(this, [safeThis, key, v]() {\n"
                     << "            if (safeThis) safeThis->updateProperty(key, v);\n"
                     << "        });\n"
                     << "        return;\n    }\n";
    }
    headerStream << "}\n\ninline void " << className << "Data::updateProperty(const QString &key, const QVariant &v) {\n";
    for (const auto &p : properties) {
        headerStream << "    if (key == " << p.propertyNameString << ") {\n"
                     << "        " << p.typeName << " nv = qvariant_cast<" << p.typeName << ">(v);\n"
                     << "        if (p_" << p.propertyName << " != nv) {\n"
                     << "            p_" << p.propertyName << " = nv;\n"
                     << "            if (m_userConfig) {\n"
                     << "                auto uc = m_userConfig;\n"
                     << "                if (uc) {\n"
                     << "                    Q_EMIT uc.data()->" << p.propertyName << "Changed();\n"
                     << "                    Q_EMIT uc.data()->valueChanged(key, v);\n"
                     << "                }\n"
                     << "            }\n"
                     << "        }\n"
                     << "        return;\n    }\n";
    }
    headerStream << "}\n\n#endif\n";

    return 0;
}
