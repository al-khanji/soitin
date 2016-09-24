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

#include <QLabel>
#include <QFileInfo>
#include <QApplication>
#include <QStyle>

#include <mediaobject.h>
#include <mediasource.h>

#include <fileref.h>
#include <tag.h>
#include <tstring.h>

#include "nowplayingbutton.h"

NowPlayingButton::NowPlayingButton(Phonon::MediaObject* mo, QWidget* parent)
: QToolButton(parent)
, m_mediaObject(mo)
{
    setText(tr("Welcome to Soitin"));
    setEnabled(false);
    setAutoRaise(true);
    setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    connect(mo, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
             this, SLOT(currentSourceChanged(Phonon::MediaSource)));
    connect(mo, SIGNAL(metaDataChanged()),
             this, SLOT(metaDataChanged()));
    connect(mo, SIGNAL(finished()),
             this, SLOT(finished()));
}

void NowPlayingButton::setText(const QString& text)
{
    QToolButton::setText(text);
    emit textChanged(this->text());
}

void NowPlayingButton::currentSourceChanged(const Phonon::MediaSource& source)
{
    setText(QFileInfo(source.fileName()).fileName());
}

void NowPlayingButton::metaDataChanged()
{
    QString title;
    QString artist;

    // Try taglib first. It gives better results.
    TagLib::FileRef fr(qPrintable(m_mediaObject->currentSource().fileName()));
    if (!fr.isNull()) {
        TagLib::Tag* tag = fr.tag();

        title = TStringToQString(tag->title());
        artist = TStringToQString(tag->artist());
    } else {
        artist = m_mediaObject->metaData(Phonon::ArtistMetaData)[0];
        title = m_mediaObject->metaData(Phonon::TitleMetaData)[0];
    }

    if (!artist.isEmpty() || !title.isEmpty()) {
        QString s = QString("%1 - %2").arg(artist).arg(title);
        setText(s);
    }
}

void NowPlayingButton::finished()
{
    setText(tr("Playlist finished"));
}
