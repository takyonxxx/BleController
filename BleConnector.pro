QT += core bluetooth positioning sensors
android|darwin: QT += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

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

ios {
    QMAKE_INFO_PLIST = ./shared/Info.qmake.ios.plist
    QMAKE_ASSET_CATALOGS = $$PWD/ios/Assets.xcassets
    QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
}

macos: QMAKE_INFO_PLIST = ./shared/Info.qmake.macos.plist

android {

}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

DISTFILES += \
    shared/Info.cmake.ios.plist \
    shared/Info.cmake.macos.plist \
    shared/Info.qmake.ios.plist
