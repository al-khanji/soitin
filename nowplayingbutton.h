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

#ifndef SOITIN_NOWPLAYINGBUTTON_H
#define SOITIN_NOWPLAYINGBUTTON_H

#include <QToolButton>
namespace Phonon {
    class MediaObject;
    class MediaSource;
}

class NowPlayingButton : public QToolButton
{
    Q_OBJECT
    public:
        NowPlayingButton(Phonon::MediaObject* mo, QWidget* parent = 0);

    public slots:
        void setText(const QString& text);

    signals:
        void textChanged(const QString& text);

    private slots:
        void currentSourceChanged(const Phonon::MediaSource& source);
        void metaDataChanged();
        void finished();

    private:
        Phonon::MediaObject* m_mediaObject;
};

#endif // SOITIN_NOWPLAYINGBUTTON_H
