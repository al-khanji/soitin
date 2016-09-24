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

#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>

#include "playlistview.h"
#include "playlistmodel.h"
#include "metadatadialog.h"

PlaylistView::PlaylistView(QWidget* parent)
: QTreeView(parent)
, m_model(0)
{}

void PlaylistView::setModel(QAbstractItemModel* model)
{
    if (model->inherits("PlaylistModel")) {
        m_model = static_cast<PlaylistModel*>(model);
    }
    QTreeView::setModel(model);
}

void PlaylistView::contextMenuEvent(QContextMenuEvent* event)
{
    if (m_model) {
        QModelIndex i = currentIndex();
        QString val = i.data().toString();
        QList<int> indexes;
        foreach (const QModelIndex& i,
            selectionModel()->selectedRows()) {
            indexes << i.row();
        }

        QMenu m;
        QAction* remove = m.addAction(tr("&Remove this Track"));
        QAction* queue = m.addAction(tr("&Queue this Track"));
        QAction* meta = m.addAction(tr("&View Track Meta Data"));
        QAction* edit = 0;
        QAction* propagate = 0;
        if (i.isValid() && (i.flags() & Qt::ItemIsEditable)) {
            m.addSeparator();
            edit = m.addAction(tr("&Edit '%1'").arg(val));
            if (indexes.count() > 1) {
                propagate = m.addAction(
                    tr("&Write '%1' for all selected tracks").arg(val)
                );
            }
        }
        QAction* result = m.exec(event->globalPos());

        if (result == remove) {
            qSort(indexes);
            while (!indexes.isEmpty()) {
                m_model->removeRow(indexes.takeLast());
            }
        } else if (result == queue) {
            foreach (int row, indexes) {
                m_model->queue(m_model->index(row, 0));
            }
        } else if (result == meta) {
            if (i.isValid()) {
                MetaDataDialog* d = new MetaDataDialog(this);
                d->setIndex(i);
                d->show();
            }
        } else if (edit != 0 && result == edit) {
            this->edit(i);
        } else if (propagate != 0 && result == propagate) {
            int column = i.column();
            QVariant data = i.data();
            foreach (int row, indexes) {
                m_model->setData(m_model->index(row, column), data);
            }
        }
    }
}

void PlaylistView::mousePressEvent(QMouseEvent* event)
{
    QTreeView::mousePressEvent(event);
    QModelIndex i = indexAt(event->pos());
    if (!(event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier))
        && (event->button() == Qt::LeftButton))
    {
        clearSelection();
        if (i.isValid()) {
            setCurrentIndex(i);
        }
    }
}

void PlaylistView::mouseReleaseEvent(QMouseEvent* event)
{
    QModelIndex i = indexAt(event->pos());
    if (i.isValid()) {
        emit clicked(i);
    }
}

void PlaylistView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QTreeView::mouseDoubleClickEvent(event);
    QModelIndex i = indexAt(event->pos());
    if (i.isValid()) {
        emit activated(i);
    }
}
