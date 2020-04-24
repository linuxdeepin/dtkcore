include($$PWD/private/private.pri)

INCLUDEPATH += $$PWD/base
INCLUDEPATH += $$PWD/private

HEADERS += \
    $$PWD/dobject.h \
    $$PWD/dsingleton.h

SOURCES += \
    $$PWD/dobject.cpp

includes.files += $$PWD/*.h
includes.files += $$PWD/private/*.h
includes.files += \
    $$PWD/DObject \
    $$PWD/DObjectPrivate \
    $$PWD/DSingleton
