HEADERS       = kgpgme.h \
    mainwindow.h \
    configdialog.h

SOURCES       = main.cpp \
                kgpgme.cpp \
    mainwindow.cpp \
    configdialog.cpp


# install
LIBS += -L/usr/lib/qca2 -lqca
LIBS += -L/usr/lib/kde4 -lkdeui
LIBS += -lgpgme
LIBS += -lkfile

INCLUDEPATH += /usr/include/QtCrypto
INCLUDEPATH += /usr/include/KDE

OTHER_FILES += \
    biomboui.rc \
    ../build/biomboui.rc \
    ../../../../.kde4/share/config/biomborc \
    TODO

FORMS += \
    configdialog.ui
