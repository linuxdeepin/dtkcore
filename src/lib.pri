TEMPLATE = lib
VERSION = 2.0

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
    DTK_INCLUDEPATH = $$PREFIX/include/libdtk-$$VERSION
} else {
    DTK_INCLUDEPATH = $$INCLUDE_INSTALL_DIR/libdtk-$$VERSION
}

win32* {
    DEFINES += STATIC_LIB
    CONFIG += staticlib
}
