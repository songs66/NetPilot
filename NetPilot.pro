QT += core gui

# 网络模块：提供TCP/UDP套接字功能
QT += network

# NDATools中后续补充的数据处理与公共模块
QT += concurrent

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
    widget.cpp \
    dataconverter.cpp \
    formchilddataconversion.cpp \
    formchilddatavalidation.cpp \
    formdataprocessor.cpp \
    inputvalidator.cpp \
    network.cpp

HEADERS += \
    errorhandler.h \
    formtcpclient.h \
    formtcpserver.h \
    formudpclient.h \
    formudpserver.h \
    widget.h \
    dataconverter.h \
    formchilddataconversion.h \
    formchilddatavalidation.h \
    formdataprocessor.h \
    inputvalidator.h \
    network.h

FORMS += \
    formtcpclient.ui \
    formtcpserver.ui \
    formudpclient.ui \
    formudpserver.ui \
    widget.ui \
    formchilddataconversion.ui \
    formchilddatavalidation.ui \
    formdataprocessor.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

DISTFILES += \
    README.md
