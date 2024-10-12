QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += .\inet

INCLUDEPATH += .\mediator

INCLUDEPATH += .\dataDef

#INCLUDEPATH += $(QT_MYDIY)/include

#LIBS += -lpthreadVC2
LIBS += -lWs2_32

LIBS += $$PWD/lib/pthreadVC2.lib

SOURCES += \
    CKernel/ckernel.cpp \
    chatdialog.cpp \
    inet/UdpNet.cpp \
    main.cpp \
    ims_dialog.cpp \
    mediator/INetMediator.cpp \
    mediator/UdpNetMediator.cpp \
    useritem.cpp

HEADERS += \
    CKernel/ckernel.h \
    chatdialog.h \
    dataDef/dataDef.h \
    ims_dialog.h \
    inet/INet.h \
    inet/UdpNet.h \
    mediator/INetMediator.h \
    mediator/UdpNetMediator.h \
    useritem.h

FORMS += \
    chatdialog.ui \
    ims_dialog.ui \
    useritem.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Res.qrc
