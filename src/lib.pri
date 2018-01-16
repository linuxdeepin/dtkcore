TEMPLATE = lib

include($$PWD/version.pri)

CONFIG += c++11 create_pc create_prl no_install_prl
CONFIG += no_keywords

DEFINES += QT_MESSAGELOGCONTEXT

isEmpty(PREFIX){
    PREFIX = /usr
}

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$PREFIX/lib
} else {
    target.path = $$LIB_INSTALL_DIR
}

isEmpty(INCLUDE_INSTALL_DIR) {
    DTK_INCLUDEPATH = $$PREFIX/include/libdtk-$${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
} else {
    DTK_INCLUDEPATH = $$INCLUDE_INSTALL_DIR/libdtk-$${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

win32* {
    DEFINES += STATIC_LIB
    CONFIG += staticlib
}

pri_dev.files += $$PWD/version.pri

isEmpty(LIB_INSTALL_DIR) {
    pri_dev.path = $$PREFIX/lib/libdtk/modules
} else {
    pri_dev.path = $$LIB_INSTALL_DIR/libdtk/modules
}

INSTALLS += pri_dev
