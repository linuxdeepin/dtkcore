QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += DTK_NO_PROJECT

# 支持从当前目录找到动态库
mac {
    QMAKE_RPATHDIR = "@executable_path"
}

SOURCES += \
    main.cpp

include($$PWD/../../src/dci/dci.pri)
INCLUDEPATH += $$PWD/../../src

#不要依赖于 dtk 的其它任何文件，要确保能在 macox 上独立编译
