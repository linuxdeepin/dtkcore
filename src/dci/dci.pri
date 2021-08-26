QT += core-private

PUBLIC_HEADERS += \
    $$PWD/ddcifile.h

HEADERS += \
    $$PUBLIC_HEADERS

SOURCES += \
    $$PWD/ddcifile.cpp

includes.files += \
    $$PUBLIC_HEADERS \
    $$PWD/DDciFile
