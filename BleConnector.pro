QT += core bluetooth sensors
android|darwin: QT += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += mobility

SOURCES += \
    gattserver.cpp \
    kalmanfilter.cpp \
    main.cpp \
    mainwindow.cpp \
    sensors.cpp

HEADERS += \
    gattserver.h \
    kalmanfilter.h \
    mainwindow.h \
    sensors.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += .

win32 {
    message("Win32 enabled")
     DEFINES += WIN32_LEAN_AND_MEAN
    RC_ICONS += $$PWD\icons\bluetooth-logo.png
}

ios {
    message("ios enabled")
    QMAKE_INFO_PLIST = ./shared/Info.qmake.ios.plist
    QMAKE_ASSET_CATALOGS = $$PWD/ios/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
}

macos: QMAKE_INFO_PLIST = ./shared/Info.qmake.macos.plist

android {
    message("android enabled")
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    shared/Info.cmake.ios.plist \
    shared/Info.cmake.macos.plist \
    shared/Info.qmake.ios.plist

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
