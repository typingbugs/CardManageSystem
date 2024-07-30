QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    VCD.cpp \
    VCDOurs.cpp \
    consumePage.cpp \
    databaseAPI.cpp \
    depositPage.cpp \
    deviceAPI.cpp \
    main.cpp \
    mainwindow.cpp \
    newCardPage.cpp \
    quitAppPage.cpp \
    readerAPI.cpp \
    reportLossPage.cpp \
    settingPage.cpp

HEADERS += \
    HF15693.h \
    HLog.h \
    VCD.h \
    VCDOurs.h \
    databaseAPI.h \
    deviceAPI.h \
    mainwindow.h \
    readerAPI.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    CardManageSystem_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    HF15693.dll \
    HF15693.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../release/ -lHF15693
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../debug/ -lHF15693
else:unix: LIBS += -L$$PWD/../ -lHF15693

LIBS += -L. -lHF15693
