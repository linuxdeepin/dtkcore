// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QProcess>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

class ut_dconfig2cpp : public testing::Test
{
protected:
    void SetUp() override {
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override {
        delete tempDir;
        tempDir = nullptr;
    }

    // Get dconfig2cpp tool path
    QString getToolPath() const {
        QStringList possiblePaths = {
            "../tools/dconfig2cpp/dconfig2cpp",
            "../../tools/dconfig2cpp/dconfig2cpp",
            "./tools/dconfig2cpp/dconfig2cpp",
            "tools/dconfig2cpp/dconfig2cpp"
        };

        for (const QString &path : possiblePaths) {
            if (QFile::exists(path)) {
                return path;
            }
        }
        return "dconfig2cpp";  // Assume it's in PATH
    }

    // Call dconfig2cpp to generate code
    struct GenerationResult {
        bool success;
        QString generatedFilePath;
        QString errorMessage;
        int exitCode;
    };

    GenerationResult generateCode(const QString& jsonFilePath) {
        GenerationResult result;
        result.success = false;
        result.exitCode = -1;

        // If it's a QRC path, copy to temporary directory first
        QString actualJsonPath = jsonFilePath;
        if (jsonFilePath.startsWith(":/")) {
            QFile sourceFile(jsonFilePath);
            if (!sourceFile.open(QIODevice::ReadOnly)) {
                result.errorMessage = QString("Failed to open resource file: %1").arg(jsonFilePath);
                return result;
            }

            QFileInfo fileInfo(jsonFilePath);
            QString tempJsonPath = tempDir->path() + "/" + fileInfo.fileName();

            QFile tempFile(tempJsonPath);
            if (!tempFile.open(QIODevice::WriteOnly)) {
                result.errorMessage = QString("Failed to create temp file: %1").arg(tempJsonPath);
                return result;
            }

            tempFile.write(sourceFile.readAll());
            tempFile.close();
            sourceFile.close();

            actualJsonPath = tempJsonPath;
        }

        if (!QFile::exists(actualJsonPath)) {
            result.errorMessage = QString("Input JSON file does not exist: %1").arg(actualJsonPath);
            return result;
        }

        QString toolPath = getToolPath();
        QProcess process;

        QFileInfo fileInfo(actualJsonPath);
        QString baseName = fileInfo.completeBaseName().replace('.', '_').replace('-', '_');
        result.generatedFilePath = tempDir->path() + "/" + baseName + ".hpp";

        QStringList arguments;
        arguments << "-o" << result.generatedFilePath << actualJsonPath;

        process.start(toolPath, arguments);

        if (!process.waitForStarted(5000)) {
            result.errorMessage = QString("Failed to start dconfig2cpp tool: %1").arg(process.errorString());
            return result;
        }

        if (!process.waitForFinished(10000)) {
            result.errorMessage = "dconfig2cpp tool execution timeout";
            process.kill();
            return result;
        }

        result.exitCode = process.exitCode();

        if (result.exitCode == 0) {
            result.success = true;
            if (!QFile::exists(result.generatedFilePath)) {
                result.success = false;
                result.errorMessage = QString("Generated file not found: %1").arg(result.generatedFilePath);
            }
        } else {
            result.errorMessage = QString("dconfig2cpp failed with exit code %1. Error: %2")
                                    .arg(result.exitCode)
                                    .arg(QString::fromUtf8(process.readAllStandardError()));
        }

        return result;
    }

    QTemporaryDir *tempDir;
};

TEST_F(ut_dconfig2cpp, BasicTypesGeneration) {
    QString testFile = ":/data/dconfig2cpp/basic-types.meta.json";

    // If resource file doesn't exist, use filesystem path
    if (!QFile::exists(testFile)) {
        testFile = "./data/dconfig2cpp/basic-types.meta.json";
    }

    ASSERT_TRUE(QFile::exists(testFile)) << "Test data file not found: " << testFile.toStdString();

    auto result = generateCode(testFile);
    ASSERT_TRUE(result.success) << result.errorMessage.toStdString();
    ASSERT_TRUE(QFile::exists(result.generatedFilePath));
}

TEST_F(ut_dconfig2cpp, BasicTypesPropertyValidation) {
    QString testFile = ":/data/dconfig2cpp/basic-types.meta.json";
    if (!QFile::exists(testFile)) {
        testFile = "./data/dconfig2cpp/basic-types.meta.json";
    }

    ASSERT_TRUE(QFile::exists(testFile));

    auto result = generateCode(testFile);
    ASSERT_TRUE(result.success) << result.errorMessage.toStdString();

    // Read generated header file content
    QFile generatedFile(result.generatedFilePath);
    ASSERT_TRUE(generatedFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString generatedContent = generatedFile.readAll();
    generatedFile.close();

    // Expected Q_PROPERTY declarations
    QStringList expectedProperties = {
        "Q_PROPERTY(bool booleanTrue READ booleanTrue WRITE setBooleanTrue",
        "Q_PROPERTY(bool booleanFalse READ booleanFalse WRITE setBooleanFalse",
        "Q_PROPERTY(QString stringValue READ stringValue WRITE setStringValue",
        "Q_PROPERTY(QString emptyString READ emptyString WRITE setEmptyString",
        "Q_PROPERTY(qlonglong integerPositive READ integerPositive WRITE setIntegerPositive",
        "Q_PROPERTY(qlonglong integerNegative READ integerNegative WRITE setIntegerNegative",
        "Q_PROPERTY(qlonglong integerZero READ integerZero WRITE setIntegerZero",
        "Q_PROPERTY(double doubleValue READ doubleValue WRITE setDoubleValue",
        "Q_PROPERTY(double doubleZero READ doubleZero WRITE setDoubleZero",
        "Q_PROPERTY(double scientificValue READ scientificValue WRITE setScientificValue"
    };

    // Verify each expected Q_PROPERTY exists
    for (const QString &expectedProperty : expectedProperties) {
        EXPECT_TRUE(generatedContent.contains(expectedProperty))
            << "Expected property not found: " << expectedProperty.toStdString();
    }
}

TEST_F(ut_dconfig2cpp, NumericTypesDetection) {
    QString testFile = ":/data/dconfig2cpp/numeric-types.meta.json";
    if (!QFile::exists(testFile)) {
        testFile = "./data/dconfig2cpp/numeric-types.meta.json";
    }

    ASSERT_TRUE(QFile::exists(testFile));

    auto result = generateCode(testFile);
    ASSERT_TRUE(result.success) << result.errorMessage.toStdString();

    // Read generated header file content
    QFile generatedFile(result.generatedFilePath);
    ASSERT_TRUE(generatedFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString generatedContent = generatedFile.readAll();
    generatedFile.close();

    // Expected Q_PROPERTY declarations - based on numeric type detection rules
    QStringList expectedProperties = {
        "Q_PROPERTY(qlonglong pureInteger READ pureInteger WRITE setPureInteger",           // Pure integer -> qlonglong
        "Q_PROPERTY(double doubleWithDecimal READ doubleWithDecimal WRITE setDoubleWithDecimal",     // Decimal -> double
        "Q_PROPERTY(double doubleWithZeroDecimal READ doubleWithZeroDecimal WRITE setDoubleWithZeroDecimal", // .0 -> double
        "Q_PROPERTY(double scientificNotation READ scientificNotation WRITE setScientificNotation", // Scientific notation -> double
        "Q_PROPERTY(qlonglong largeInteger READ largeInteger WRITE setLargeInteger"        // Large integer -> qlonglong
    };

    // Verify each expected Q_PROPERTY exists
    for (const QString &expectedProperty : expectedProperties) {
        EXPECT_TRUE(generatedContent.contains(expectedProperty))
            << "Expected property not found: " << expectedProperty.toStdString();
    }
}

TEST_F(ut_dconfig2cpp, ComplexTypesGeneration) {
    QString testFile = ":/data/dconfig2cpp/complex-types.meta.json";
    if (!QFile::exists(testFile)) {
        testFile = "./data/dconfig2cpp/complex-types.meta.json";
    }

    ASSERT_TRUE(QFile::exists(testFile));

    auto result = generateCode(testFile);
    ASSERT_TRUE(result.success) << result.errorMessage.toStdString();

    // Read generated header file content
    QFile generatedFile(result.generatedFilePath);
    ASSERT_TRUE(generatedFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString generatedContent = generatedFile.readAll();
    generatedFile.close();

    // Expected Q_PROPERTY declarations - complex types
    QStringList expectedProperties = {
        "Q_PROPERTY(QList<QVariant> emptyArray READ emptyArray WRITE setEmptyArray",
        "Q_PROPERTY(QList<QVariant> stringArray READ stringArray WRITE setStringArray",
        "Q_PROPERTY(QList<QVariant> mixedArray READ mixedArray WRITE setMixedArray",
        "Q_PROPERTY(QVariantMap emptyObject READ emptyObject WRITE setEmptyObject",
        "Q_PROPERTY(QVariantMap simpleObject READ simpleObject WRITE setSimpleObject"
    };

    // Verify each expected Q_PROPERTY exists
    for (const QString &expectedProperty : expectedProperties) {
        EXPECT_TRUE(generatedContent.contains(expectedProperty))
            << "Expected property not found: " << expectedProperty.toStdString();
    }
}
