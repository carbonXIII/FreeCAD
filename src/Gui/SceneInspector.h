/***************************************************************************
 *   Copyright (c) 2008 Werner Mayer <wmayer[at]users.sourceforge.net>     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef GUI_SCENEINSPECTOR_H
#define GUI_SCENEINSPECTOR_H

#include <QDialog>
#include <QHash>
#include <QTreeWidget>
#include "InventorBase.h"

class SoNode;

namespace Gui {
class Document;
class PrefWidgetStates;
namespace Dialog {

class Ui_SceneInspector;
class DlgInspector;

/// Stores data representing scenegraph nodes.
class SceneModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    SceneModel(QObject* parent);
    ~SceneModel() override;

    /** returns empty QVariant, unless orientation == Qt::Horizontal,
     *  role == Qt::DisplayRole and section == 0 where it returns
     *  "Inventor Tree"
     */
    QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    /// header data not used: returns false
    bool setHeaderData (int section, Qt::Orientation orientation, const QVariant & value, int role = Qt::EditRole) override;
    /// insert the first node in tree
    void setNode(SoNode* node);
    /// set names per node
    void setNodeNames(const QHash<SoNode*, QString>& names);

    QModelIndex parent(const QModelIndex & index) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int, const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &) const override;

private:

    friend class DlgInspector;

    struct Item {
        QModelIndex parent;
        CoinPtr<SoNode> node;
        bool expand = true;
    };
    mutable QMap<QModelIndex, Item> items;
    Item rootItem;

    QHash<SoNode*, QString> nodeNames;
    bool autoExpanding = false;
};

/// Dialog window to display scenegraph model as a tree
class DlgInspector : public QDialog
{
    Q_OBJECT

public:
    DlgInspector(QWidget* parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());
    ~DlgInspector() override;

    void setDocument(Gui::Document* doc);

private:
    void onRefreshButtonClicked();
    void onTreeViewPressed(const QModelIndex &);
    void onFieldViewItemExpanded(QTreeWidgetItem *item);

protected:
    void changeEvent(QEvent *e) override;
    void setNode(SoNode* node);
    void setNodeNames(Gui::Document*);
    void populateFieldView(QTreeWidgetItem *parent, SoNode *n);
    void expandItem(QTreeWidgetItem *item, bool force = false);
    void initExpand();
    void contextMenuEvent(QContextMenuEvent *);

private:
    Ui_SceneInspector* ui;
    std::unique_ptr<Gui::PrefWidgetStates> widgetStates;
};

} // namespace Dialog
} // namespace Gui

#endif // GUI_SCENEINSPECTOR_H
