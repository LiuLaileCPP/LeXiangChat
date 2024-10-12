QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

greaterThan(QT_MAJOR_VERSION,5):QT += core5compat
DEFINES +="_HAS_STD_BYTE=0"

INCLUDEPATH += .\inet

INCLUDEPATH += .\mediator

INCLUDEPATH += .\dataDef

INCLUDEPATH += .\CKernel
#INCLUDEPATH += $(QT_MYDIY)/include

#LIBS += -lpthreadVC2
LIBS += -lWs2_32

#pragma comment(lib,"winmm.lib")
LIBS += -lwinmm

LIBS += $$PWD/lib/pthreadVC2.lib

SOURCES += \
    CKernel/ckernel.cpp \
    chatdialog.cpp \
    groupchatdialog.cpp \
    groupitem.cpp \
    inet/INet.cpp \
    inet/TcpClient.cpp \
    inet/UdpNet.cpp \
    logindialog.cpp \
    main.cpp \
    ims_dialog.cpp \
    mediator/INetMediator.cpp \
    mediator/TcpClientMediator.cpp \
    mediator/UdpNetMediator.cpp \
    memberitem.cpp \
    noconnectdialog.cpp \
    useritem.cpp

HEADERS += \
    CKernel/ckernel.h \
    chatdialog.h \
    dataDef/dataDef.h \
    groupchatdialog.h \
    groupitem.h \
    ims_dialog.h \
    inet/INet.h \
    inet/TcpClient.h \
    inet/UdpNet.h \
    logindialog.h \
    mediator/INetMediator.h \
    mediator/TcpClientMediator.h \
    mediator/UdpNetMediator.h \
    memberitem.h \
    noconnectdialog.h \
    useritem.h

FORMS += \
    chatdialog.ui \
    groupchatdialog.ui \
    groupitem.ui \
    ims_dialog.ui \
    logindialog.ui \
    memberitem.ui \
    noconnectdialog.ui \
    useritem.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Res.qrc
