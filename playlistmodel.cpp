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


#include <QTime>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>
#include <QMimeData>
#include <QUrl>

#include <mediaobject.h>
#include <mediasource.h>

#include <fileref.h>
#include <tag.h>
#include <audioproperties.h>
#include <tstring.h>

#include "playlistmodel.h"
#include "utils.h"

struct TrackInfo {
    TrackInfo(const QString& filename)
    : filename(filename)
    , length(-1)
    , bitRate(0)
    , sampleRate(0)
    , channels(0)
    , title(QFileInfo(filename).fileName())
    , year(0)
    , track(0)
    {}

    QString filename;

    // AudioProperties data
    int length;
    int bitRate;
    int sampleRate;
    int channels;

    // Tag data
    QString title;
    QString artist;
    QString album;
    QString comment;
    QString genre;
    uint year;
    uint track;
};

static const char* const SoitinMime = "application/x-soitin-tracks";

PlaylistModel::PlaylistModel(Phonon::MediaObject* mediaObject, QObject* parent)
: QAbstractItemModel(parent)
, m_mediaObject(mediaObject)
, m_shuffle(false)
, m_current(0)
{
    setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);
    connect(m_mediaObject, SIGNAL(aboutToFinish()),
             this, SLOT(queueNext()));
    connect(m_mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
             this, SLOT(currentSourceChanged(Phonon::MediaSource)));
    connect(m_mediaObject, SIGNAL(finished()),
             this, SLOT(finished()));
    connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
             this, SLOT(stateChanged(Phonon::State, Phonon::State)));
}

int PlaylistModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return COLUMN_SENTINEL + 1;
}

void PlaylistModel::addFile(const QString& file, int row)
{
    TagLib::FileRef fr(file.toLocal8Bit().constData());
    TrackInfo* ti = new TrackInfo(file);

    if (!fr.isNull()) {
        TagLib::AudioProperties* ap = fr.audioProperties();
        TagLib::Tag* tag = fr.tag();

        if (ap) {
            ti->length = ap->length();
            ti->bitRate = ap->bitrate();
            ti->sampleRate = ap->sampleRate();
            ti->channels = ap->channels();
        }

        ti->title = TStringToQString(tag->title());
        ti->artist = TStringToQString(tag->artist());
        ti->album = TStringToQString(tag->album());
        ti->comment = TStringToQString(tag->comment());
        ti->genre = TStringToQString(tag->genre());
        ti->year = tag->year();
        ti->track = tag->track();
    }

    if (row == -1) {
        row = m_tracks.count();
    }

    beginInsertRows(QModelIndex(), row, row);
    m_tracks.insert(row, ti);
    endInsertRows();
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= m_tracks.count() || index.isValid() != true) {
        return QVariant();
    }

    QVariant result;
    const TrackInfo& ti = *m_tracks[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case Artist:
                result = ti.artist;
                break;
            case Album:
                result = ti.album;
                break;
            case Title:
                result = ti.title;
                break;
            case Track:
                if (ti.track > 0) {
                    result = ti.track;
                }
                break;
            case Length:
                if (ti.length >= 0) {
                    result = Utils::formatSeconds(ti.length);
                }
                break;
            case BitRate:
                result = ti.bitRate;
                break;
            case SampleRate:
                result = ti.sampleRate;
                break;
            case Channels:
                result = ti.channels;
                break;
            case Comment:
                result = ti.comment;
                break;
            case Genre:
                result = ti.genre;
                break;
            case Year:
                result = ti.year;
                break;
            case Filename:
                result = ti.filename;
                break;
            default:
                result = tr("Internal Error");
                break;
        }
    } else if (role == Qt::FontRole) {
        switch (index.column()) {
            case Length:
                result = QFont("Monospace");
                break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case Album:
            case Artist:
            case Track:
                result = Qt::AlignCenter;
                break;
            case Length:
                result = Qt::AlignRight;
                break;
            default:
                result = Qt::AlignLeft;
                break;
        }
    } else if (role == Qt::BackgroundRole) {
        if (&ti == m_current) {
            QBrush b = QPalette().highlight();
            result = QBrush(b.color().lighter());
        }
    } else if (role == Qt::ForegroundRole) {
        if (&ti == m_current) {
            result = QPalette().highlightedText();
        }
    }

    return result;
}

QModelIndex PlaylistModel::index(int row, int column,
                                  const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    QModelIndex result;
    if (row >= 0 && row < m_tracks.count()) {
        result = createIndex(row, column, m_tracks[row]);
    }
    return result;
}

QModelIndex PlaylistModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);

    return QModelIndex(); // we are a flat model
}

int PlaylistModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_tracks.count();
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    Q_UNUSED(orientation);
    QVariant result;
    if (role == Qt::DisplayRole) {
        switch (section) {
            case Artist:
                result = tr("Artist");
                break;
            case Album:
                result = tr("Album");
                break;
            case Title:
                result = tr("Title");
                break;
            case Track:
                result = tr("Track");
                break;
            case Length:
                result = tr("Length");
                break;
            default:
                result = tr("Internal Error");
                break;
        }
    } else if (role == Qt::TextAlignmentRole) {
        result = Qt::AlignCenter;
    }
    return result;
}

class Sorter {
    private:
        int m_c;
        Qt::SortOrder m_o;

    public:
        Sorter(int column, Qt::SortOrder o) : m_c(column), m_o(o) {}
        bool operator()(const TrackInfo* const t1, const TrackInfo* const t2) const {
            int res = 0;

            switch (m_c) {
                case PlaylistModel::Artist:
                    res = t1->artist.localeAwareCompare(t2->artist);
                    break;
                case PlaylistModel::Album:
                    res = t1->album.localeAwareCompare(t2->album);
                    break;
                case PlaylistModel::Title:
                    res = t1->title.localeAwareCompare(t2->title);
                    break;
                case PlaylistModel::Track:
                    res = t1->track - t2->track;
                    break;
                case PlaylistModel::Length:
                    res = t1->length - t2->length;
                    break;
            }

            res = m_o == Qt::AscendingOrder ? -res : res;
            return res < 0;
        }
};

void PlaylistModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    qStableSort(m_tracks.begin(), m_tracks.end(), Sorter(column, order));

    foreach (const QModelIndex& from, persistentIndexList()) {
        TrackInfo* ti = static_cast<TrackInfo*>(from.internalPointer());
        QModelIndex to = createIndex(m_tracks.indexOf(ti),
                                        from.column(),
                                        ti);
        changePersistentIndex(from, to);
    }

    emit layoutChanged();
}

Qt::DropActions PlaylistModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions()
           | Qt::CopyAction
           | Qt::MoveAction
           | Qt::LinkAction;
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index)
                          | Qt::ItemIsDragEnabled
                          | Qt::ItemIsDropEnabled;
    switch (index.column()) {
        case Title:
        case Album:
        case Artist:
        case Year:
        case Track:
        case Comment:
        case Genre:
            flags |= Qt::ItemIsEditable;
        default:
            break; // shut up gcc
    }

    return flags;
}

QStringList PlaylistModel::mimeTypes() const
{
    QStringList types = QAbstractItemModel::mimeTypes();
    types << "text/plain" << "text/uri-list" << SoitinMime;
    return types;
}

QMimeData* PlaylistModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mime = new QMimeData;

    QStringList text;
    QList<QUrl> urls;

    QSet<int> rows; // we get multiple columns per row, process only one
    QVariantList idx;

    foreach (const QModelIndex& index, indexes) {
        if (rows.contains(index.row())) {
            continue;
        }

        if (index.isValid()) {
            int row = index.row();
            const TrackInfo* const ti = m_tracks[row];
            QString artist = ti->artist;
            QString title = ti->title;
            QString filename = ti->filename;

            if (artist.isEmpty() && title.isEmpty()) {
                text << filename;
            } else {
                text << QString("%1 - %2").arg(artist).arg(title);
            }

            urls << QUrl::fromLocalFile(ti->filename);
            idx << row;
            rows << row;
        }
    }

    mime->setText(text.join("\n"));
    mime->setUrls(urls);

    QByteArray ba;
    QDataStream str(&ba, QIODevice::WriteOnly);
    str << idx;
    mime->setData(SoitinMime, ba);

    return mime;
}

bool PlaylistModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                 int row, int column, const QModelIndex& parent)
{
    Q_UNUSED(column);

    if (parent.isValid() && row == -1) {
        row = parent.row() + 1;
    }

    bool result;

    if (data->hasFormat(SoitinMime)) {
        // this is a bit tricky, pay attention
        QByteArray ba = data->data(SoitinMime);
        QDataStream str(&ba, QIODevice::ReadOnly);
        QVariantList variants;
        QList<TrackInfo*> tracks;
        QList<int> ints;
        TrackInfo* ti = 0;
        if (row >= 0) {
            ti = m_tracks[row];
        }
        str >> variants;
        foreach (const QVariant& v, variants) {
            bool ok = false;
            int i = v.toInt(&ok);
            if (ok) {
                ints << i;
            }
        }
        qSort(ints);
        // good, boiler plate stuff done...
        // what we do is remove rows in decreasing order so the indexes are
        // valid, then insert the tracks (in correct order) where we want them.
        // the removal happens in two loops, first we get the TrackInfo pointers
        // using the list indexes, then we remove them. clumsy... and brittle.
        while (!ints.isEmpty()) {
            int i = ints.takeLast();
            tracks << m_tracks[i];
        }
        foreach (TrackInfo* ti, tracks) {
            int i = m_tracks.indexOf(ti);
            beginRemoveRows(QModelIndex(), i, i);
            m_tracks.removeAll(ti);
            endRemoveRows();
        }
        row = m_tracks.indexOf(ti);
        while (!tracks.isEmpty()) {
            TrackInfo* ti = tracks.takeLast();
            addFile(ti->filename, row);
            delete ti;
        }
        result = true;
    } else if (data->hasUrls()) {
        foreach (const QUrl& url, data->urls()) {
            addFile(url.toString(), row);
        }
        result = true;
    } else if (action == Qt::IgnoreAction ){
        result = false;
    } else {
        result = false;
    }

    return result;
}

bool PlaylistModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row >= 0 && row < m_tracks.count() && !parent.isValid()) {
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int i = 0; i < count; i++) {
            delete m_tracks.takeAt(row);
        }
        endRemoveRows();
        return true;
    }
    return false;
}

bool PlaylistModel::setData(const QModelIndex& index, const QVariant& value,
                             int role)
{
    Q_UNUSED(role);

    TrackInfo* ti = m_tracks[index.row()];
    TagLib::FileRef fr(qPrintable(ti->filename));
    bool success = false;

    if (!fr.isNull()) {
        TagLib::Tag* t = fr.tag();

        switch (index.column()) {
            case Title:
                t->setTitle(qPrintable(value.toString()));
                ti->title = value.toString();
                break;
            case Artist:
                t->setArtist(qPrintable(value.toString()));
                ti->artist = value.toString();
                break;
            case Album:
                t->setAlbum(qPrintable(value.toString()));
                ti->album = value.toString();
                break;
            case Comment:
                t->setComment(qPrintable(value.toString()));
                ti->comment = value.toString();
                break;
            case Genre:
                t->setGenre(qPrintable(value.toString()));
                ti->genre = value.toString();
                break;
            case Year:
                t->setYear(value.toUInt());
                ti->year = value.toUInt();
                break;
            case Track:
                t->setTrack(value.toUInt());
                ti->track = value.toUInt();
                break;
        }

        success = fr.save();
    }

    if (success) {
        emit dataChanged(index, index);
    }

    return success;
}

bool PlaylistModel::shuffle() const
{
    return m_shuffle;
}

QModelIndex PlaylistModel::currentIndex() const
{
    QModelIndex res;
    if (m_current) {
        res = index(m_tracks.indexOf(m_current), 0);
    }
    return res;
}

Phonon::MediaObject* PlaylistModel::mediaObject() const
{
    return m_mediaObject;
}

void PlaylistModel::queue(const QModelIndex& index)
{
    m_queue.enqueue(m_tracks[index.row()]);
}

void PlaylistModel::loadPlaylist(QTextStream& stream)
{
    QLatin1Char comment('#');
    QString line;
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.contains(comment)) {
            line.truncate(line.indexOf(comment));
        }
        line = line.trimmed();
        if (line.count() > 0) {
            addFile(line);
        }
    }
}

void PlaylistModel::savePlaylist(QTextStream& stream) const
{
    foreach (TrackInfo* ti, m_tracks) {
        qDebug("Writing %s", qPrintable(ti->filename));
        stream << ti->filename << '\n';
    }
}

void PlaylistModel::play(const QModelIndex& index)
{
    if (index.isValid()) {
        m_mediaObject->setCurrentSource(m_tracks[index.row()]->filename);
    } else {
        queueNext();
    }
    m_mediaObject->play();
}

void PlaylistModel::setShuffle(bool shuffle)
{
    m_shuffle = shuffle;
}

void PlaylistModel::previous()
{
    if (m_mediaObject->currentTime() < Q_INT64_C(10000)) {
        if (TrackInfo* ti = previousTrack()) {
            play(index(m_tracks.indexOf(ti), 0));
        }
    } else {
        m_mediaObject->seek(Q_INT64_C(0));
    }
}

void PlaylistModel::next()
{
    if (TrackInfo* ti = nextTrack()) {
        play(index(m_tracks.indexOf(ti), 0));
    } else {
        m_mediaObject->seek(m_mediaObject->totalTime());
    }
}

void PlaylistModel::clear()
{
    foreach (TrackInfo* ti, m_tracks) {
        delete ti;
    }
    m_tracks.clear();
    reset();
}

void PlaylistModel::queueNext()
{
    if (TrackInfo* ti = nextTrack()) {
        m_mediaObject->enqueue(ti->filename);
    }
}

void PlaylistModel::currentSourceChanged(const Phonon::MediaSource& newSource)
{
    for (int i = 0; i < m_tracks.count(); i++) {
        TrackInfo* ti = m_tracks[i];
        if (ti->filename == newSource.fileName()) {
            int j = qMax(0, m_tracks.indexOf(m_current));
            QModelIndex topLeft = index(qMin(i, j), 0);
            QModelIndex bottomRight = index(qMax(i, j), COLUMN_SENTINEL);
            m_current = ti;
            emit dataChanged(topLeft, bottomRight);
            break;
        }
    }
}

void PlaylistModel::finished()
{
    int i = m_tracks.indexOf(m_current);
    m_current = 0;
    emit dataChanged(index(i, 0), index(i, COLUMN_SENTINEL));
}

void PlaylistModel::stateChanged(Phonon::State newState,
                                  Phonon::State oldState)
{
    Q_UNUSED(oldState);
    if (newState == Phonon::ErrorState) {
        if (m_mediaObject->errorType() == Phonon::FatalError) {
            next();
        }
    }
}

TrackInfo* PlaylistModel::nextTrack()
{
    TrackInfo* ti = 0;
    int next = -1;

    if (!m_queue.isEmpty()) {
        return m_queue.dequeue();
    }

    if (m_shuffle) {
        next = qreal(qrand()) / qreal(RAND_MAX) * qreal(m_tracks.count());
    } else {
        if (m_current) {
            next = m_tracks.indexOf(m_current) + 1;
        } else {
            next = 0;
        }
    }

    if (m_tracks.count() > 0 && next >= 0 && next < m_tracks.count()) {
        ti = m_tracks[next];
    }

    return ti;
}

TrackInfo* PlaylistModel::previousTrack()
{
    TrackInfo* ti = 0;
    int current = m_tracks.indexOf(m_current);
    if (current > 0) {
        ti = m_tracks[current-1];
    }
    return ti;
}
