HEADERS += \
    $$PWD/dutil.h \
    $$PWD/dpinyin.h \
    $$PWD/dtimeunitformatter.h \
    $$PWD/dabstractunitformatter.h \
    $$PWD/ddisksizeformatter.h

includes.files += $$PWD/*.h
includes.files += \
    $$PWD/DUtil \
    $$PWD/DPinyin

RESOURCES += \
    $$PWD/util.qrc

SOURCES += \
    $$PWD/dtimeunitformatter.cpp \
    $$PWD/dabstractunitformatter.cpp \
    $$PWD/ddisksizeformatter.cpp
