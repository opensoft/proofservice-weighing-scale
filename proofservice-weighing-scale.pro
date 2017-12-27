TARGET = proofservice-weighing-scale
TEMPLATE = app

VERSION = 0.17.12.25

macx:QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig proofnetwork c++14
PKGCONFIG += libusb-1.0

SOURCES += \
    main.cpp \
    weighingscalerestserver.cpp \
    weighingscalehandler.cpp \
    3rdparty/hidapi/hid.c

HEADERS += \
    weighingscalerestserver.h \
    weighingscalehandler.h \
    proofservice-weighing-scale_global.h \
    3rdparty/hidapi/hidapi.h

DISTFILES += \
    CHANGELOG.md \
    UPGRADE.md \
    README.md


include($$(PROOF_PATH)/proof_service.pri)
