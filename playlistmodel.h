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

#ifndef SOITIN_PLAYLISTMODEL_H
#define SOITIN_PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QQueue>
#include <phononnamespace.h>
class QTextStream;

namespace Phonon {
    class MediaObject;
    class MediaSource;
}

struct TrackInfo;

class PlaylistModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS(Column)
    public:
        PlaylistModel(Phonon::MediaObject* mediaObject, QObject* parent = 0);

        void addFile(const QString& file, int row = -1);

        // pure virtuals
        int columnCount(const QModelIndex& parent = QModelIndex()) const;
        QVariant data(const QModelIndex& index,
                       int role = Qt::DisplayRole) const;
        QModelIndex index(int row, int column,
                           const QModelIndex& parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex& index) const;
        int rowCount(const QModelIndex& parent = QModelIndex()) const;

        // virtuals
        QVariant headerData(int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole) const;
        void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
        Qt::DropActions supportedDropActions() const;
        Qt::ItemFlags flags(const QModelIndex& index) const;
        QStringList mimeTypes() const;
        QMimeData* mimeData(const QModelIndexList& indexes) const;
        bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                           int row, int column, const QModelIndex& parent);
        bool removeRows(int row, int count,
                         const QModelIndex& parent = QModelIndex());
        bool setData(const QModelIndex& index, const QVariant& value,
                      int role = Qt::EditRole);

        // NOTE! This is important.
        // If you add any new data columns be sure to update the Sorter class.
        // Otherwise you might get interesting results when trying to sort that
        // column.
        enum Column {
            Title,
            Artist,
            Album,
            Track,
            Length,
            BitRate,
            SampleRate,
            Channels,
            Comment,
            Genre,
            Year,
            Filename,
            COLUMN_SENTINEL = Length // Last entry to show
        };

        bool shuffle() const;
        QModelIndex currentIndex() const;
        Phonon::MediaObject* mediaObject() const;
        void queue(const QModelIndex& index);
        void loadPlaylist(QTextStream& stream);
        void savePlaylist(QTextStream& stream) const;

    public slots:
        void play(const QModelIndex& index = QModelIndex());
        void setShuffle(bool shuffle);
        void previous();
        void next();
        void clear();

    private slots:
        void queueNext();
        void currentSourceChanged(const Phonon::MediaSource& newSource);
        void finished();
        void stateChanged(Phonon::State newState, Phonon::State oldState);

    private:
        QList<TrackInfo*> m_tracks;
        QQueue<TrackInfo*> m_queue;
        Phonon::MediaObject* m_mediaObject;
        bool m_shuffle;
        TrackInfo* m_current;

        TrackInfo* nextTrack();
        TrackInfo* previousTrack();
};

#endif // SOITIN_PLAYLISTMODEL_H
