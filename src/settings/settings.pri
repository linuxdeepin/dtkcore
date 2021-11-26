linux {
    PKGCONFIG += gsettings-qt

SOURCES += \
    $$PWD/backend/gsettingsbackend.cpp

HEADERS +=\
    $$PWD/backend/gsettingsbackend.h
}

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/backend/qsettingbackend.cpp \
    $$PWD/dsettings.cpp \
    $$PWD/dsettingsoption.cpp \
    $$PWD/dsettingsgroup.cpp \
    $$PWD/backend/dsettingsdconfigbackend.cpp

HEADERS +=\
    $$PWD/backend/qsettingbackend.h \
    $$PWD/dsettings.h \
    $$PWD/dsettingsoption.h \
    $$PWD/dsettingsgroup.h \
    $$PWD/dsettingsbackend.h \
    $$PWD/backend/dsettingsdconfigbackend.h

includes.files += $${PWD}/*.h
includes.files += $${PWD}/backend/*.h
includes.files += \
    $${PWD}/DSettings \
    $${PWD}/DSettingsGroup \
    $${PWD}/DSettingsOption
