HEADERS += \
    $$PWD/dutil.h \
    $$PWD/dpinyin.h \
    $$PWD/dtimeunitformatter.h \
    $$PWD/dabstractunitformatter.h \
    $$PWD/ddisksizeformatter.h \
    $$PWD/ddbussender.h \
    $$PWD/drecentmanager.h \
    $$PWD/dnotifysender.h \
    $$PWD/dexportedinterface.h \
    $$PWD/dvtablehook.h \
    $$PWD/dfileservices.h

INCLUDEPATH += $$PWD

includes.files += $$PWD/*.h
includes.files += \
    $$PWD/DUtil \
    $$PWD/DPinyin \
    $$PWD/DDBusSender \
    $$PWD/DRecentManager \
    $$PWD/DNotifySender \
    $$PWD/DExportedInterface \
    $$PWD/DVtableHook \
    $$PWD/DFileServices

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
    $$PWD/dexportedinterface.cpp \
    $$PWD/dvtablehook.cpp

linux {
    QT += dbus

    SOURCES += \
        $$PWD/dfileservices_linux.cpp
} else {
    SOURCES += \
        $$PWD/dfileservices_dummy.cpp
}
