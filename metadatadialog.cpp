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

#include "metadatadialog.h"
#include "playlistmodel.h"
#include "utils.h"

MetaDataDialog::MetaDataDialog(QWidget* parent)
: QDialog(parent)
{
    setupUi(this);
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
             this, SLOT(clicked(QAbstractButton*)));
}

void MetaDataDialog::setIndex(const QModelIndex& index)
{
    m_index = index;
    reset();
}

void MetaDataDialog::clicked(QAbstractButton* button)
{
    switch (buttonBox->buttonRole(button)) {
        case QDialogButtonBox::ResetRole:
            reset();
            break;
        case QDialogButtonBox::AcceptRole:
            apply();
            break;
        default:
            break; // shut up gcc
    }
}

void MetaDataDialog::reset()
{
    QString title, artist, album, comment, genre, filename, length;
    int bitRate, sampleRate, channels;
    uint year, track;

    bitRate = sampleRate = channels = year = track = 0;

    if (m_index.isValid()) {
        const QAbstractItemModel* m_model = m_index.model();
        int row = m_index.row();
        title = m_model->data(m_model->index(row, PlaylistModel::Title)).toString();
        artist = m_model->data(m_model->index(row, PlaylistModel::Artist)).toString();
        album = m_model->data(m_model->index(row, PlaylistModel::Album)).toString();
        comment = m_model->data(m_model->index(row, PlaylistModel::Comment)).toString();
        genre = m_model->data(m_model->index(row, PlaylistModel::Genre)).toString();
        length = m_model->data(m_model->index(row, PlaylistModel::Length)).toString();
        bitRate = m_model->data(m_model->index(row, PlaylistModel::BitRate)).toInt();
        sampleRate = m_model->data(m_model->index(row, PlaylistModel::SampleRate)).toInt();
        channels = m_model->data(m_model->index(row, PlaylistModel::Channels)).toInt();
        year = m_model->data(m_model->index(row, PlaylistModel::Year)).toUInt();
        track = m_model->data(m_model->index(row, PlaylistModel::Track)).toUInt();
        filename = m_model->data(m_model->index(row, PlaylistModel::Filename)).toString();
    }

    titleEdit->setText(title);
    artistEdit->setText(artist);
    albumEdit->setText(album);
    commentEdit->setText(comment);
    genreEdit->setText(genre);

    filenameLabel->setText(filename);
    lengthLabel->setText(length);
    bitRateLabel->setText(tr("%1 kb/s").arg(QString::number(bitRate)));
    sampleRateLabel->setText(tr("%1 Hz").arg(QString::number(sampleRate)));
    yearSpinBox->setValue(year);
    trackSpinBox->setValue(track);
}

void MetaDataDialog::apply()
{
    if (m_index.isValid()) {
        QAbstractItemModel* m_model = 0;
        // bleh
        m_model = const_cast<QAbstractItemModel*>(m_index.model());
        int row = m_index.row();
        m_model->setData(m_model->index(row, PlaylistModel::Title),
                          titleEdit->text());
        m_model->setData(m_model->index(row, PlaylistModel::Artist),
                          artistEdit->text());
        m_model->setData(m_model->index(row, PlaylistModel::Album),
                          albumEdit->text());
        m_model->setData(m_model->index(row, PlaylistModel::Comment),
                          commentEdit->text());
        m_model->setData(m_model->index(row, PlaylistModel::Genre),
                          genreEdit->text());
        m_model->setData(m_model->index(row, PlaylistModel::Year),
                          yearSpinBox->value());
        m_model->setData(m_model->index(row, PlaylistModel::Track),
                          trackSpinBox->value());
    }
}
