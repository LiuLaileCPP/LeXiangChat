QT       += core gui
QT       += core gui
QT       += core5compat
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

greaterThan(QT_MAJOR_VERSION,5):QT += core5compat

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES +="_HAS_STD_BYTE=0"

#LIBS += -lpthreadVC2
LIBS += -lWs2_32

LIBS += -lwinmm

LIBS += $$PWD/lib/pthreadVC2.lib

SOURCES += \
    chatdlg.cpp \
    config/tools.cpp \
    group.cpp \
    item.cpp \
    main.cpp \
    kernel.cpp \
    maindlg.cpp \
    net/inet.cpp \
    net/tcpnet.cpp \
    net/udpnet.cpp \
    person.cpp \
    reglogdlg.cpp \
    udpthread.cpp \
    wd_msg.cpp \
    wd_msgme.cpp

HEADERS += \
    chatdlg.h \
    config/Cli_config.h \
    config/tools.h \
    group.h \
    item.h \
    kernel.h \
    maindlg.h \
    net/inet.h \
    net/tcpnet.h \
    net/udpnet.h \
    person.h \
    reglogdlg.h \
    udpthread.h \
    wd_msg.h \
    wd_msgme.h

FORMS += \
    chatdlg.ui \
    item.ui \
    kernel.ui \
    maindlg.ui \
    reglogdlg.ui \
    wd_msg.ui \
    wd_msgme.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
