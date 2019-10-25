HEADERS += \
    $$PWD/dutil.h \
    $$PWD/dpinyin.h \
    $$PWD/dtimeunitformatter.h \
    $$PWD/dabstractunitformatter.h \
    $$PWD/ddisksizeformatter.h \
    $$PWD/ddbussender.h \
    $$PWD/drecentmanager.h \
    $$PWD/dnotifysender.h \
    $$PWD/dexportedinterface.h

INCLUDEPATH += $$PWD

includes.files += $$PWD/*.h
includes.files += \
    $$PWD/DUtil \
    $$PWD/DPinyin \
    $$PWD/DDBusSender \
    $$PWD/DRecentManager \
    $$PWD/DNotifySender \
    $$PWD/DExportedInterface

RESOURCES += \
    $$PWD/util.qrc

SOURCES += \
    $$PWD/dtimeunitformatter.cpp \
    $$PWD/dabstractunitformatter.cpp \
    $$PWD/ddisksizeformatter.cpp \
    $$PWD/ddbussender.cpp \
    $$PWD/drecentmanager.cpp \
    $$PWD/dnotifysender.cpp \
    $$PWD/dpinyin.cpp \
    $$PWD/dexportedinterface.cpp
