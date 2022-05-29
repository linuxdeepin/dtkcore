HEADERS += \
    $$PWD/epoll/epoll_global.h \
    $$PWD/epoll/private/qcore_unix_p.h \
    $$PWD/epoll/private/qtimerinfo_unix_p.h

SOURCES += \
    $$PWD/epoll/private/qcore_unix.cpp \
    $$PWD/epoll/private/qtimerinfo_unix.cpp

INCLUDEPATH += $$PWD/epoll $$PWD/epoll/private
