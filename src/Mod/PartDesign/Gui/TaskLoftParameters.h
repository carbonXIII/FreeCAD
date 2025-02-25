/***************************************************************************
 *   Copyright (c) 2015 Stefan Tröger <stefantroeger@gmx.net>              *
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

#ifndef GUI_TASKVIEW_TaskLoftParameters_H
#define GUI_TASKVIEW_TaskLoftParameters_H

#include <boost_signals2.hpp>

#include "TaskSketchBasedParameters.h"
#include "ViewProviderLoft.h"


class Ui_TaskLoftParameters;
class QListWidget;
class QListWidgetItem;

namespace App {
class Property;
}

namespace Gui {
class ViewProvider;
}

namespace PartDesignGui {


class TaskLoftParameters : public TaskSketchBasedParameters
{
    Q_OBJECT

public:
    explicit TaskLoftParameters(ViewProviderLoft *LoftView, bool newObj=false, QWidget *parent = nullptr);
    ~TaskLoftParameters() override;

private Q_SLOTS:
    void onClosed(bool);
    void onRuled(bool);
    void onSplitProfile(bool);
    void onMaxDegree(int);
    void updateUI();

protected:
    void refresh() override;

private:
    QWidget* proxy;
    std::unique_ptr<Ui_TaskLoftParameters> ui;
    boost::signals2::scoped_connection connSections;
    LinkSubListWidget *sectionWidget;
};

/// simulation dialog for the TaskView
class TaskDlgLoftParameters : public TaskDlgSketchBasedParameters
{
    Q_OBJECT

public:
    explicit TaskDlgLoftParameters(ViewProviderLoft *LoftView,bool newObj=false);
    ~TaskDlgLoftParameters() override;

    ViewProviderLoft* getLoftView() const
    { return static_cast<ViewProviderLoft*>(vp); }

protected:
    TaskLoftParameters  *parameter;
};

} //namespace PartDesignGui

#endif // GUI_TASKVIEW_TASKAPPERANCE_H
