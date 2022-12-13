// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtextencoding.h"

#include <gtest/gtest.h>

#include <QLibrary>
#include <QFile>
#include <QTextCodec>

DCORE_USE_NAMESPACE

class ut_DTextEncoding : public testing::Test
{
public:
    static void SetUpTestCase();

    bool rewriteTempFile(const QByteArray &data);
    void removeTempFile();

    static bool canLoadUchardet;
    static bool canLoadICU;
    static QString tmpFileName;

    static QByteArray dataGB18030;
    static QByteArray dataEUC_JP;
    static QByteArray dataKOI8_R;

protected:
    void TearDown() override;
};

bool ut_DTextEncoding::canLoadUchardet = false;
bool ut_DTextEncoding::canLoadICU = false;
QString ut_DTextEncoding::tmpFileName = QString("/tmp/ut_DTextEncoding_temp.txt");
QByteArray ut_DTextEncoding::dataGB18030;
QByteArray ut_DTextEncoding::dataEUC_JP;
QByteArray ut_DTextEncoding::dataKOI8_R;

void ut_DTextEncoding::SetUpTestCase()
{
    QLibrary uchardet("libuchardet", "0");
    if (!uchardet.isLoaded()) {
        canLoadUchardet = uchardet.load();
        if (canLoadUchardet) {
            uchardet.unload();
        }
    }

    QLibrary icuuc("libicuuc");
    if (icuuc.isLoaded()) {
        canLoadICU = icuuc.load();
        if (canLoadICU) {
            icuuc.unload();
        }
    }

    // Utf8 Text: 中文测试一二三四123456789abcdefgh
    const QByteArray chineseUnicode("\u4e2d\u6587\u6d4b\u8bd5\u4e00\u4e8c\u4e09\u56db\u0031\u0032\u0033\u0034\u0035\u0036\u0037"
                                    "\u0038\u0039\u0061\u0062\u0063\u0064\u0065\u0066\u0067\u0068");
    // Utf8 Text: 日本語のテスト ワン ツー スリー フォー 123456789abcdefgh
    const QByteArray japaneseUnicode(
        "\u65e5\u672c\u8a9e\u306e\u30c6\u30b9\u30c8\u0020\u30ef\u30f3\u0020\u30c4\u30fc\u0020\u30b9\u30ea\u30fc\u0020\u30d5\u30a9"
        "\u30fc\u0020\u0031\u0032\u0033\u0034\u0035\u0036\u0037\u0038\u0039\u0061\u0062\u0063\u0064\u0065\u0066\u0067\u0068");
    // Utf8 Text: Русский тест раз два три четыре 123456789abcdefgh
    const QByteArray russianUnicode(
        "\u0420\u0443\u0441\u0441\u043a\u0438\u0439\u0020\u0442\u0435\u0441\u0442\u0020\u0440\u0430\u0437\u0020\u0434\u0432\u0430"
        "\u0020\u0442\u0440\u0438\u0020\u0447\u0435\u0442\u044b\u0440\u0435\u0020\u0031\u0032\u0033\u0034\u0035\u0036\u0037\u0038"
        "\u0039\u0061\u0062\u0063\u0064\u0065\u0066\u0067\u0068");

    QTextCodec *codec = QTextCodec::codecForName("GB18030");
    if (codec) {
        dataGB18030 = codec->fromUnicode(QString::fromUtf8(chineseUnicode));
    }
    codec = QTextCodec::codecForName("EUC-JP");
    if (codec) {
        dataEUC_JP = codec->fromUnicode(QString::fromUtf8(japaneseUnicode));
    }
    codec = QTextCodec::codecForName("KOI8-R");
    if (codec) {
        dataKOI8_R = codec->fromUnicode(QString::fromUtf8(russianUnicode));
    }
}

bool ut_DTextEncoding::rewriteTempFile(const QByteArray &data)
{
    QFile tmpFile(tmpFileName);
    if (!tmpFile.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    tmpFile.write(data);
    tmpFile.close();
    return true;
}

void ut_DTextEncoding::removeTempFile()
{
    if (QFile::exists(tmpFileName)) {
        QFile::remove(tmpFileName);
    }
}

void ut_DTextEncoding::TearDown()
{
    removeTempFile();
}

TEST_F(ut_DTextEncoding, testDetectTextEncode)
{
    // Default encoding is utf-8.
    ASSERT_EQ("UTF-8", DTextEncoding::detectTextEncoding(""));
    ASSERT_EQ("UTF-8", DTextEncoding::detectTextEncoding("12345678ABCDEFG"));

    ASSERT_EQ("GB18030", DTextEncoding::detectTextEncoding(dataGB18030));
    ASSERT_EQ("EUC-JP", DTextEncoding::detectTextEncoding(dataEUC_JP));
    ASSERT_EQ("KOI8-R", DTextEncoding::detectTextEncoding(dataKOI8_R));
}

TEST_F(ut_DTextEncoding, testDetectTextEncodeWithUchardet)
{
    if (canLoadUchardet) {
        QByteArray uchardetEncoding("EUC-TW");
        // QTextCodec not suppotted EUC-TW.
        QTextCodec *codec = QTextCodec::codecForName(uchardetEncoding);
        ASSERT_EQ(codec, nullptr);

        // Utf8 text: 繁體中文測試一二三四
        QByteArray dataZhTraditional("\u7e41\u9ad4\u4e2d\u6587\u6e2c\u8a66\u4e00\u4e8c\u4e09\u56db");
        QByteArray dataEUC_TW;
        ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataZhTraditional, dataEUC_TW, uchardetEncoding));
        ASSERT_EQ(uchardetEncoding, DTextEncoding::detectTextEncoding(dataEUC_TW));

        QByteArray convertGB18030;
        ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataEUC_TW, convertGB18030, "GB18030"));
        QByteArray convertUTF8;
        ASSERT_TRUE(DTextEncoding::convertTextEncoding(convertGB18030, convertUTF8, "UTF-8"));
        ASSERT_EQ(dataZhTraditional, convertUTF8);
    }
}

TEST_F(ut_DTextEncoding, testDetectFileEncode)
{
    bool isOk = false;
    ASSERT_EQ("", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_FALSE(isOk);

    ASSERT_TRUE(rewriteTempFile(""));
    ASSERT_EQ("UTF-8", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_TRUE(isOk);

    ASSERT_TRUE(rewriteTempFile(dataGB18030));
    ASSERT_EQ("GB18030", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_TRUE(isOk);

    ASSERT_TRUE(rewriteTempFile(dataEUC_JP));
    ASSERT_EQ("EUC-JP", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_TRUE(isOk);

    ASSERT_TRUE(rewriteTempFile(dataKOI8_R));
    ASSERT_EQ("KOI8-R", DTextEncoding::detectFileEncoding(tmpFileName, &isOk));
    ASSERT_TRUE(isOk);
}

TEST_F(ut_DTextEncoding, testConvertTextEncoding)
{
    QByteArray dataUTF_8;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataGB18030, dataUTF_8, "UTF-8"));
    ASSERT_EQ("UTF-8", DTextEncoding::detectTextEncoding(dataUTF_8));
    QTextCodec *codec = QTextCodec::codecForName("GB18030");
    ASSERT_EQ(codec->toUnicode(dataGB18030).toUtf8(), dataUTF_8);

    QByteArray convertedGB18030;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_8, convertedGB18030, "GB18030"));
    ASSERT_EQ(dataGB18030, convertedGB18030);

    // Convert with multi bytes encoding.
    QByteArray dataUTF_16;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_8, dataUTF_16, "UTF-16"));
    ASSERT_EQ("UTF-16", DTextEncoding::detectTextEncoding(dataUTF_16));
    codec = QTextCodec::codecForName("UTF-16");
    ASSERT_EQ(codec->toUnicode(dataUTF_16).toUtf8(), dataUTF_8);

    QByteArray convertedUTF8;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_16, convertedUTF8, "UTF-8"));
    ASSERT_EQ(dataUTF_8, convertedUTF8);

    QByteArray dataUTF_32;
    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_8, dataUTF_32, "UTF-32"));
    ASSERT_EQ("UTF-32", DTextEncoding::detectTextEncoding(dataUTF_32));
    codec = QTextCodec::codecForName("UTF-32");
    ASSERT_EQ(codec->toUnicode(dataUTF_32).toUtf8(), dataUTF_8);

    ASSERT_TRUE(DTextEncoding::convertTextEncoding(dataUTF_16, convertedUTF8, "UTF-8"));
    ASSERT_EQ(dataUTF_8, convertedUTF8);
}

TEST_F(ut_DTextEncoding, testConvertTextEncodingWithError)
{
    QByteArray dataUTF_8;
    ASSERT_FALSE(DTextEncoding::convertTextEncoding(dataGB18030, dataUTF_8, "ERROR"));
    ASSERT_FALSE(DTextEncoding::convertTextEncoding(dataGB18030, dataUTF_8, "KOI8-R"));
}

TEST_F(ut_DTextEncoding, testConvertFileEncoding)
{
    ASSERT_TRUE(rewriteTempFile(dataGB18030));
    ASSERT_TRUE(DTextEncoding::convertFileEncoding(tmpFileName, "UTF-8"));
    ASSERT_EQ("UTF-8", DTextEncoding::detectFileEncoding(tmpFileName));

    ASSERT_TRUE(DTextEncoding::convertFileEncoding(tmpFileName, "UTF-32"));
    ASSERT_EQ("UTF-32", DTextEncoding::detectFileEncoding(tmpFileName));

    ASSERT_FALSE(DTextEncoding::convertFileEncoding("", "UTF-32"));
}

TEST_F(ut_DTextEncoding, testConvertFileEncodingTo)
{
    QString tmpConvertFileName("/tmp/ut_DTextEncoding_temp_testConvertFileEncodingTo.txt");
    ASSERT_TRUE(rewriteTempFile(dataGB18030));
    ASSERT_TRUE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "GB18030"));
    ASSERT_TRUE(QFile::exists(tmpConvertFileName));

    ASSERT_TRUE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "UTF-8"));
    ASSERT_TRUE(QFile::exists(tmpConvertFileName));
    ASSERT_EQ("UTF-8", DTextEncoding::detectFileEncoding(tmpConvertFileName));

    ASSERT_TRUE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "UTF-32"));
    ASSERT_EQ("UTF-32", DTextEncoding::detectFileEncoding(tmpConvertFileName));

    ASSERT_TRUE(QFile::remove(tmpConvertFileName));
}

TEST_F(ut_DTextEncoding, testConvertFileEncodingToWithError)
{
    QString tmpConvertFileName("/tmp/ut_DTextEncoding_temp_testConvertFileEncodingToWithError.txt");
    ASSERT_FALSE(DTextEncoding::convertFileEncodingTo("", tmpConvertFileName, "UTF-32"));
    ASSERT_FALSE(DTextEncoding::convertFileEncodingTo(tmpFileName, "", "UTF-32"));
    ASSERT_FALSE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "ERROR"));
    ASSERT_FALSE(QFile::exists(tmpConvertFileName));

    ASSERT_TRUE(rewriteTempFile(dataGB18030));
    ASSERT_FALSE(DTextEncoding::convertFileEncodingTo(tmpFileName, tmpConvertFileName, "EUC-JP"));
    ASSERT_FALSE(QFile::exists(tmpConvertFileName));
}
