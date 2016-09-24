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

#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QHeaderView>
#include <QToolButton>
#include <QSettings>
#include <QDebug>

#include <mediaobject.h>
#include <audiooutput.h>
#include <seekslider.h>
#include <volumeslider.h>
#include <videowidget.h>

#include "mainwindow.h"
#include "playlistmodel.h"
#include "nowplayingbutton.h"
#include "utils.h"

#include <stdio.h>

MainWindow::MainWindow(PlaylistModel* model, QWidget* parent)
: QMainWindow(parent)
, m_model(model)
, m_mediaObject(model->mediaObject())
, m_audioOutput(0)
, m_seekSlider(0)
, m_volumeSlider(0)
, m_videoWidget(0)
, m_showActions(0)
, m_nowPlayingButton(0)
{
    setupUi(this);
    setWindowIcon(QIcon(Utils::directory(Utils::Pixmaps) + "/soitin-64.png"));

    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    m_videoWidget = new Phonon::VideoWidget(this);
    Phonon::createPath(m_mediaObject, m_audioOutput);
    Phonon::createPath(m_mediaObject, m_videoWidget);
    m_videoWidget->installEventFilter(this);

    stackedWidget->addWidget(treeView);
    stackedWidget->addWidget(m_videoWidget);
    stackedWidget->setCurrentWidget(treeView);

    treeView->setModel(m_model);

    m_volumeSlider = new Phonon::VolumeSlider(m_audioOutput, this);
    statusBar()->insertWidget(0, m_volumeSlider);

    m_seekSlider = new Phonon::SeekSlider(m_mediaObject, this);
    statusBar()->insertWidget(0, m_seekSlider);

    m_nowPlayingButton = new NowPlayingButton(m_mediaObject, this);
    statusBar()->insertWidget(0, m_nowPlayingButton);

    m_showActions = new QActionGroup(this);
    m_showActions->addAction(actionShowVideo);
    m_showActions->addAction(actionShowPlaylist);

    setupActions();
    connect(treeView, SIGNAL(activated(QModelIndex)),
             m_model, SLOT(play(QModelIndex)));
    connect(m_mediaObject, SIGNAL(hasVideoChanged(bool)),
             this, SLOT(checkVideo()));
    connect(m_mediaObject, SIGNAL(finished()),
             this, SLOT(finished()));
    connect(m_mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
             this, SLOT(currentSourceChanged()));
    connect(m_nowPlayingButton, SIGNAL(clicked()),
             this, SLOT(jumpToCurrent()));
    connect(m_nowPlayingButton, SIGNAL(textChanged(QString)),
             this, SLOT(setWindowTitle(QString)));

    restoreSession();
    frobLayout(false);
}

void MainWindow::addFiles(QStringList files)
{
    if (files.isEmpty()) {
        files = QFileDialog::getOpenFileNames(this,tr("Select Tracks"));
    }
    if (!files.isEmpty()) {
        foreach (QString f, files) {
            m_model->addFile(f);
        }
    }
    frobLayout();
}

void MainWindow::addDir(QString path)
{
    bool top = false;
    if (path.isEmpty()) {
        path = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
        top = true;
    }

    if (!path.isEmpty()) {
        QDir dir(path);
        if (!dir.isReadable()) {
            QMessageBox::critical(this,
                tr("Unreadable Directory"),
                tr("The directory %1 is not readable. Sorry.").arg(path));
            return;
        } else if (!dir.exists()) {
            QMessageBox::critical(this,
                tr("Nonexistant Directory"),
                tr("The directory %1 does not exist. Sorry.").arg(path));
            return;
        }
        foreach (const QString& f, dir.entryList(QDir::Files |
                                                 QDir::Readable)) {
            m_model->addFile(dir.filePath(f));
        }
        foreach (const QString& f, dir.entryList(QDir::Dirs |
                                                 QDir::Readable |
                                                 QDir::NoDotAndDotDot)) {
            qApp->processEvents();
            addDir(dir.filePath(f));
        }
        if (top) {
            frobLayout();
        }
    }
}

void MainWindow::play()
{
    if (m_model->rowCount() > 0) {
        m_model->play();
    }
}

void MainWindow::setShuffle(bool shuffle)
{
    actionShuffle->setChecked(shuffle);
}

void MainWindow::frobLayout(bool sort)
{
    if (sort) {
        Qt::SortOrder order = Qt::DescendingOrder;
        treeView->sortByColumn(PlaylistModel::Track, order);
        treeView->sortByColumn(PlaylistModel::Album, order);
        treeView->sortByColumn(PlaylistModel::Artist, order);
    }
    treeView->header()->resizeSections(QHeaderView::ResizeToContents);
    treeView->resizeColumnToContents(PlaylistModel::Track);
}

void MainWindow::commitData()
{
    qDebug() << "Saving session";
    QByteArray b;
    QTextStream t(&b, QIODevice::WriteOnly);
    m_model->savePlaylist(t);
    t.flush();
    QSettings s;
    s.setValue("session", b);
    qDebug() << b;
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_videoWidget) {
        switch (event->type()) {
            case QEvent::MouseButtonDblClick:
                actionFullScreen->toggle();
                break;
            default:
                break; // stop gcc complaining about unhandled enum values
        }
    }
    return false;
}

void MainWindow::checkVideo()
{
    if (m_mediaObject->hasVideo()) {
        actionShowVideo->setChecked(true);
        actionFullScreen->setEnabled(true);
    } else {
        actionShowPlaylist->setChecked(true);
        actionFullScreen->setChecked(false);
        actionFullScreen->setEnabled(false);
    }
}

void MainWindow::showPlaylist(bool show)
{
    if (show) {
        stackedWidget->setCurrentWidget(treeView);
    }
}

void MainWindow::showVideo(bool show)
{
    if (show) {
        stackedWidget->setCurrentWidget(m_videoWidget);
    }
}

void MainWindow::finished()
{
    actionShowPlaylist->setChecked(true);
    m_nowPlayingButton->setEnabled(false);
    actionPlay->setChecked(false);
}

void MainWindow::about()
{
    QMessageBox::about(this,
        tr("About"),
        tr("<html>Soitin %2 - Music for regular People"
           "<br>Copyright &copy; 2008 "
           "<a href=\"mailto:louai.khanji@gmail.com\">Louai Al-Khanji</a>"
           "<br>Bug reports welcome, patches even more so."
           "</html>").arg(SOITIN_VERSION));
}

void MainWindow::currentSourceChanged()
{
    checkVideo();
    m_nowPlayingButton->setEnabled(true);
    actionPlay->setChecked(true);
}

void MainWindow::jumpToCurrent()
{
    treeView->scrollTo(m_model->currentIndex());
}

void MainWindow::play(bool p)
{
    if (p) {
        m_mediaObject->play();
    } else {
        m_mediaObject->pause();
    }
}

void MainWindow::stop()
{
    actionPlay->setChecked(false);
    m_mediaObject->stop();
}

void MainWindow::showFullScreen(bool show)
{
    if (show) {
        actionShowVideo->setChecked(true);
    }
    m_videoWidget->setFullScreen(show);
}

void MainWindow::open(QString filename)
{
    if (filename.isEmpty()) {
        filename = QFileDialog::getOpenFileName(
            this,
            tr("Select Playlist"),
            QString(),
            tr("Playlists (*.m3u)")
        );
    }

    if (!filename.isEmpty()) {
        QFile f(filename);
        if (f.exists() && f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&f);
            m_model->loadPlaylist(in);
            frobLayout(false);
        }
    }
}

void MainWindow::save()
{
    saveAs(m_current);
}

void MainWindow::saveAs(QString filename)
{
    if (filename.isEmpty()) {
        filename = QFileDialog::getSaveFileName(
            this,
            tr("Save Playlist"),
            QString(),
            tr("Playlists (*.m3u)")
        );
    }

    if (!filename.isEmpty()) {
        QFile f(filename);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&f);
            m_model->savePlaylist(out);
            m_current = filename;
        }
    }
}

void MainWindow::restoreSession()
{
    qDebug("Restoring session");
    QSettings s;
    QByteArray b = s.value("session", QVariant()).toByteArray();
    qDebug() << b;
    if (!b.isEmpty()) {
        QTextStream t(&b);
        m_model->loadPlaylist(t);
    }
}

void MainWindow::setupActions()
{
    actionPrevious->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    actionPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    actionStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    actionNext->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));

    connect(actionAddFiles, SIGNAL(triggered()),
             this, SLOT(addFiles()));
    connect(actionAddFolder, SIGNAL(triggered()),
             this, SLOT(addDir()));
    connect(actionShuffle, SIGNAL(toggled(bool)),
             m_model, SLOT(setShuffle(bool)));
    connect(actionQuit, SIGNAL(triggered()),
             this, SLOT(close()));
    connect(actionShowVideo, SIGNAL(toggled(bool)),
             this, SLOT(showVideo(bool)));
    connect(actionShowPlaylist, SIGNAL(toggled(bool)),
             this, SLOT(showPlaylist(bool)));
    connect(actionPrevious, SIGNAL(triggered()),
             m_model, SLOT(previous()));
    connect(actionNext, SIGNAL(triggered()),
             m_model, SLOT(next()));
    connect(actionAbout, SIGNAL(triggered()),
             this, SLOT(about()));
    connect(actionPlay, SIGNAL(toggled(bool)),
             this, SLOT(play(bool)));
    connect(actionStop, SIGNAL(triggered()),
             this, SLOT(stop()));
    connect(actionFullScreen, SIGNAL(toggled(bool)),
             this, SLOT(showFullScreen(bool)));
    connect(actionClear, SIGNAL(triggered()),
             m_model, SLOT(clear()));
    connect(actionOpen, SIGNAL(triggered()),
             this, SLOT(open()));
    connect(actionSave, SIGNAL(triggered()),
             this, SLOT(save()));
    connect(actionSaveAs, SIGNAL(triggered()),
             this, SLOT(saveAs()));
}
