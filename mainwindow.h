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

#ifndef SOITIN_MAINWINDOW_H
#define SOITIN_MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
class PlaylistModel;
class QActionGroup;
class NowPlayingButton;
class QToolButton;

namespace Phonon {
    class MediaObject;
    class AudioOutput;
    class SeekSlider;
    class VolumeSlider;
    class VideoWidget;
}

// TODO: the api sucks, make it good.
class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
    public:
        MainWindow(PlaylistModel* model, QWidget* parent = 0);

    public slots:
        void addFiles(QStringList files = QStringList());
        void addDir(QString path = QString());
        void play(); // Plays first track in list
        void setShuffle(bool shuffle);
        void frobLayout(bool sort = true);
        void commitData();

    protected:
        bool eventFilter(QObject* watched, QEvent* event);

    private slots:
        void checkVideo();
        void showPlaylist(bool show);
        void showVideo(bool show);
        void finished();
        void about();
        void currentSourceChanged();
        void jumpToCurrent();
        void play(bool p);
        void stop();
        void showFullScreen(bool show);
        void open(QString filename = QString());
        void save();
        void saveAs(QString filename = QString());
        void restoreSession();

    private:
        PlaylistModel* m_model;
        Phonon::MediaObject* m_mediaObject;
        Phonon::AudioOutput* m_audioOutput;
        Phonon::SeekSlider* m_seekSlider;
        Phonon::VolumeSlider* m_volumeSlider;
        Phonon::VideoWidget* m_videoWidget;
        QActionGroup* m_showActions;
        NowPlayingButton* m_nowPlayingButton;
        QString m_current;

        void setupActions();
};

#endif // SOITIN_MAINWINDOW_H
