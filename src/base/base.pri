include($$PWD/private/private.pri)

HEADERS += \
    $$PWD/dobject.h \
    $$PWD/dsingleton.h

SOURCES += \
    $$PWD/dobject.cpp

includes.files += $$PWD/*.h
includes.files += $$PWD/*.cpp
includes.files += \
    $$PWD/DObject \
    $$PWD/DSingleton
