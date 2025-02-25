/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *   Copyright (c) 2014 WandererFan <wandererfan@gmail.com>                *
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

#ifndef DRAWINGGUI_QGRAPHICSITEMVIEWANNOTATION_H
#define DRAWINGGUI_QGRAPHICSITEMVIEWANNOTATION_H

#include <Mod/TechDraw/TechDrawGlobal.h>

#include "QGIView.h"

namespace TechDraw {
class DrawViewAnnotation;
}

namespace TechDrawGui
{
class QGCustomText;

class TechDrawGuiExport QGIViewAnnotation : public QGIView
{
    Q_OBJECT
public:

    explicit QGIViewAnnotation();

    /// m_textItem belongs to this group and will be deleted by Qt
    ~QGIViewAnnotation() override = default;

    enum {Type = QGraphicsItem::UserType + 120};
    int type() const override { return Type;}

    void updateView(bool update = false) override;
    void setViewAnnoFeature(TechDraw::DrawViewAnnotation *obj);

    void draw() override;
    void rotateView() override;

    void setPreselect(bool enable) override;

protected:
    void drawAnnotation();

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    QGCustomText *m_textItem;
    QColor m_colNormal;
    QColor m_colSel;
    QColor m_colPre;
    bool m_hasHover = false;
};

} // end namespace TechDrawGui

#endif // DRAWINGGUI_QGRAPHICSITEMVIEWANNOTATION_H
