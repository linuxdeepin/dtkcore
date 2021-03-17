TARGET = qdbusxml2cpp-fix

TEMPLATE = app
QT += core dbus-private dbus
CONFIG += c++11

SOURCES += qdbusxml2cpp.cpp

host_sw_64 {
    QMAKE_CXXFLAGS += -mieee
}

#target.path = $$TOOL_INSTALL_DIR
target.path = $$PREFIX/bin

INSTALLS += target
