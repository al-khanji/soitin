# Copyright (C) 2008 Louai Al-Khanji <louai.khanji@gmail.com>
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
# claim that you wrote the original software. If you use this software
# in a product, an acknowledgment in the product documentation would be
# appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
# misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.


# install prefix
unix {
    PREFIX=/usr
}
!unix {
    PREFIX=.
}

# where to install stuff under the prefix
TRANSLATIONS_DIR = $$PREFIX/share/soitin
ICON_DIR = $$PREFIX/share/pixmaps
DESKTOP_DIR = $$PREFIX/share/applications
DOCS_DIR = $$PREFIX/share/doc/soitin
BIN_DIR = $$PREFIX/bin

CONFIG += link_pkgconfig
TEMPLATE = app
TARGET = soitin
DEPENDPATH += .
INCLUDEPATH += .
QT += phonon
PKGCONFIG += taglib
VERSION = 1.1.1
DEFINES += SOITIN_VERSION=\\\"$$VERSION\\\" \
           TRANSLATIONS_DIR=\\\"$$TRANSLATIONS_DIR\\\" \
           PIXMAPS_DIR=\\\"$$ICON_DIR\\\"
QMAKE_DISTCLEAN += *.qm

# Input
HEADERS += mainwindow.h playlistmodel.h nowplayingbutton.h playlistview.h \
           metadatadialog.h utils.h
FORMS += mainwindow.ui metadatadialog.ui
SOURCES += main.cpp mainwindow.cpp playlistmodel.cpp nowplayingbutton.cpp \
           playlistview.cpp metadatadialog.cpp utils.cpp
TRANSLATIONS = soitin_pl.ts \
               soitin_fi.ts

unix {
    system("lrelease *.pro")
}

target.path = $$BIN_DIR
icon.path = $$ICON_DIR
icon.files = *.png
desktop.path = $$DESKTOP_DIR
desktop.files = soitin.desktop
translations.path = $$TRANSLATIONS_DIR
translations.files = *.qm
docs.path = $$DOCS_DIR
docs.files = CHANGES LICENSING README
INSTALLS += target icon desktop translations docs
