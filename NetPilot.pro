QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    errorhandler.cpp \
    formtcpclient.cpp \
    formtcpserver.cpp \
    formudpclient.cpp \
    formudpserver.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    errorhandler.h \
    formtcpclient.h \
    formtcpserver.h \
    formudpclient.h \
    formudpserver.h \
    widget.h

FORMS += \
    formtcpclient.ui \
    formtcpserver.ui \
    formudpclient.ui \
    formudpserver.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

DISTFILES += \
    README.md
