/*

Copyright (C) 2008 Louai Al-Khanji <louai.khanji@gmail.com>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

#include <iostream>

#include <QApplication>
#include <QtGlobal>
#include <QDateTime>
#include <QTranslator>
#include <QLocale>
#include <QMetaObject>

#include <mediaobject.h>

#include "mainwindow.h"
#include "playlistmodel.h"
#include "utils.h"

QStringList shuffle = QStringList() << "--shuffle" << "-s";
QStringList help = QStringList() << "--help" << "-h";
QStringList verbose = QStringList() << "--verbose" << "-v";
QStringList version = QStringList() << "--version" << "-V";

bool debug = false;

static void messageHandler(QtMsgType type, const char* msg)
{
    switch (type) {
        case QtDebugMsg:
            if (debug) {
                std::cerr << "Debug: " << msg << std::endl;
            }
            break;
        case QtWarningMsg:
            std::cerr << "Warning: " << msg << std::endl;
            break;
        case QtCriticalMsg:
            std::cerr << "Critical: " << msg << std::endl;
            break;
        case QtFatalMsg:
            std::cerr << "Fatal: " << msg << std::endl;
            break;
    }
}

static void printHelp(const QString& appName)
{
    QString sep = ", ";
    QString h = QString("%1 <options> <files>\n\n").arg(appName);
    h += "Valid Arguments:\n";
    h += QString("\t%1\t: shuffle tracks\n").arg(shuffle.join(sep));
    h += QString("\t%1\t: show this help\n").arg(help.join(sep));
    h += QString("\t%1\t: verbose output\n").arg(verbose.join(sep));
    h += QString("\t%1\t: print version\n").arg(version.join(sep));
    std::cout << qPrintable(h) << std::endl;
}

static void printVersion()
{
    std::cout << SOITIN_VERSION << std::endl;
}

int main(int argc, char** argv)
{
    // Qt has an annoying habit nowadays of spurging out lots of text that is
    // completely useless to the end-user. Make it saner.
    qInstallMsgHandler(messageHandler);

    QApplication app(argc, argv);
    bool doShuffle = false;

    QStringList args = app.arguments();
    foreach (QString s, help) {
        if (args.contains(s)) {
            args.removeAll(s);
            printHelp(args[0]);
            return 0;
        }
    }
    foreach (QString s, version) {
        if (args.contains(s)) {
            args.removeAll(s);
            printVersion();
            return 0;
        }
    }
    foreach (QString s, shuffle) {
        if (args.contains(s)) {
            args.removeAll(s);
            doShuffle = true;
        }
    }
    foreach (QString s, verbose) {
        if (args.contains(s)) {
            args.removeAll(s);
            debug = true;
        }
    }

    app.setApplicationName("Soitin");
    app.setApplicationVersion(SOITIN_VERSION);
    app.setOrganizationName("Project Therapy");

    qsrand(QDateTime::currentDateTime().toTime_t());

    QString translationsDir = Utils::directory(Utils::Translations);
    QString locale = QLocale::system().name();
    QString translation = "soitin_" + locale;
    QTranslator translator;
    qDebug("Loading translation %s", qPrintable(translation));
    if (!translator.load(translation, translationsDir)) {
        qDebug("Failed to load translation for locale: %s",
                  qPrintable(locale));
    }
    app.installTranslator(&translator);

    Phonon::MediaObject mediaObject;
    PlaylistModel playlistModel(&mediaObject);
    MainWindow w(&playlistModel);
    w.setShuffle(doShuffle);
    w.show();

    QObject::connect(&app, SIGNAL(aboutToQuit()),
                      &w, SLOT(commitData()));

    args.removeAt(0); // remove app name
    QStringList files;
    QStringList dirs;

    foreach (const QString& arg, args) {
        QFileInfo fi(arg);
        if (fi.isFile()) {
            files << arg;
        } else if (fi.isDir()) {
            dirs << arg;
        }
    }

    // We use meta object magic because we want to run these things after
    // entering the main application loop

    if (!files.isEmpty()) {
        QMetaObject::invokeMethod(&w, "addFiles", Qt::QueuedConnection,
                                   Q_ARG(QStringList, files));
    }

    if (!dirs.isEmpty()) {
        foreach (const QString& s, dirs) {
            QMetaObject::invokeMethod(&w, "addDir", Qt::QueuedConnection,
                                       Q_ARG(QString, s));
        }
    }

    QMetaObject::invokeMethod(&w, "play", Qt::QueuedConnection);
    QMetaObject::invokeMethod(&w, "frobLayout", Qt::QueuedConnection);
    return app.exec();
}
