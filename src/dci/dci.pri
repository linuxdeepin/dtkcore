QT += core-private

PUBLIC_HEADERS += \
    $$PWD/ddcifile.h

HEADERS += \
    $$PUBLIC_HEADERS \
    $$PWD/private/ddcifileengine_p.h

SOURCES += \
    $$PWD/ddcifile.cpp \
    $$PWD/private/ddcifileengine.cpp

includes.files += \
    $$PUBLIC_HEADERS \
    $$PWD/DDciFile
