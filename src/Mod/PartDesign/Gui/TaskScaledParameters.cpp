/******************************************************************************
 *   Copyright (c) 2012 Jan Rheinländer <jrheinlaender@users.sourceforge.net> *
 *                                                                            *
 *   This file is part of the FreeCAD CAx development system.                 *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Library General Public              *
 *   License as published by the Free Software Foundation; either             *
 *   version 2 of the License, or (at your option) any later version.         *
 *                                                                            *
 *   This library  is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU Library General Public License for more details.                     *
 *                                                                            *
 *   You should have received a copy of the GNU Library General Public        *
 *   License along with this library; see the file COPYING.LIB. If not,       *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,            *
 *   Suite 330, Boston, MA  02111-1307, USA                                   *
 *                                                                            *
 ******************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
# include <QAction>
#endif

#include <App/DocumentObject.h>
#include <Base/Console.h>
#include <Base/ExceptionSafeCall.h>
#include <Base/Tools.h>
#include <Gui/Command.h>
#include <Gui/Selection.h>
#include <Gui/SelectionObject.h>
#include <Gui/SpinBox.h>
#include <Gui/ViewProvider.h>
#include <Mod/PartDesign/App/FeatureScaled.h>

#include "ui_TaskScaledParameters.h"
#include "TaskScaledParameters.h"
#include "TaskMultiTransformParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskScaledParameters */

TaskScaledParameters::TaskScaledParameters(ViewProviderTransformed *TransformedView,QWidget *parent)
    : TaskTransformedParameters(TransformedView, parent)
    , ui(new Ui_TaskScaledParameters)
{
    // we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui->setupUi(proxy);

    this->groupLayout()->addWidget(proxy);

    ui->buttonOK->hide();
    ui->checkBoxUpdateView->setEnabled(true);

    ui->labelOccurrences->hide();
    ui->spinOccurrences->hide();

    blockUpdate = false; // Hack, sometimes it is NOT false although set to false in Transformed::Transformed()!!
    setupUI();
}

TaskScaledParameters::TaskScaledParameters(TaskMultiTransformParameters *parentTask, QLayout *layout)
        : TaskTransformedParameters(parentTask), ui(new Ui_TaskScaledParameters)
{
    proxy = new QWidget(parentTask);
    ui->setupUi(proxy);
    connect(ui->buttonOK, &QPushButton::pressed,
            parentTask, &TaskScaledParameters::onSubTaskButtonOK);

    layout->addWidget(proxy);

    ui->buttonOK->setEnabled(true);
    ui->checkBoxUpdateView->hide();

    blockUpdate = false; // Hack, sometimes it is NOT false although set to false in Transformed::Transformed()!!
    setupUI();
}

void TaskScaledParameters::setupUI()
{
    setupBaseUI();

    Base::connect(ui->spinFactor, QOverload<double>::of(&Gui::QuantitySpinBox::valueChanged),
            this, &TaskScaledParameters::onFactor);
    Base::connect(ui->spinOccurrences, QOverload<uint>::of(&Gui::UIntSpinBox::unsignedChanged),
            this, &TaskScaledParameters::onOccurrences);
    Base::connect(ui->checkBoxUpdateView, &QCheckBox::toggled,
            this, &TaskScaledParameters::onUpdateView);
    // ---------------------

    PartDesign::Scaled* pcScaled = static_cast<PartDesign::Scaled*>(getObject());
    ui->spinFactor->bind(pcScaled->Factor);
    ui->spinOccurrences->setMaximum(INT_MAX);
    ui->spinOccurrences->bind(pcScaled->Occurrences);
    ui->spinFactor->setEnabled(true);
    ui->spinOccurrences->setEnabled(true);
    //ui->spinFactor->setDecimals(Base::UnitsApi::getDecimals());

    updateUI();
}

void TaskScaledParameters::updateUI()
{
    Base::StateLocker lock(blockUpdate);

    PartDesign::Scaled* pcScaled = static_cast<PartDesign::Scaled*>(getObject());

    double factor = pcScaled->Factor.getValue();
    unsigned occurrences = pcScaled->Occurrences.getValue();

    ui->spinFactor->setValue(factor);
    ui->spinOccurrences->setValue(occurrences);
}

void TaskScaledParameters::onFactor(const double f)
{
    if (blockUpdate)
        return;
    setupTransaction();
    PartDesign::Scaled* pcScaled = static_cast<PartDesign::Scaled*>(getObject());
    pcScaled->Factor.setValue(f);
    recomputeFeature();
}

void TaskScaledParameters::onOccurrences(const uint n)
{
    if (blockUpdate)
        return;
    setupTransaction();
    PartDesign::Scaled* pcScaled = static_cast<PartDesign::Scaled*>(getObject());
    pcScaled->Occurrences.setValue(n);
    recomputeFeature();
}

void TaskScaledParameters::onUpdateView(bool on)
{
    blockUpdate = !on;
    if (on) {
        // Do the same like in TaskDlgScaledParameters::accept() but without doCommand
        setupTransaction();
        PartDesign::Scaled* pcScaled = static_cast<PartDesign::Scaled*>(getObject());
        pcScaled->Factor.setValue(getFactor());
        pcScaled->Occurrences.setValue(getOccurrences());
        recomputeFeature();
    }
}

double TaskScaledParameters::getFactor(void) const
{
    return ui->spinFactor->value().getValue();
}

unsigned TaskScaledParameters::getOccurrences(void) const
{
    return ui->spinOccurrences->value();
}

TaskScaledParameters::~TaskScaledParameters()
{
    if (proxy)
        delete proxy;
}

void TaskScaledParameters::changeEvent(QEvent *e)
{
    TaskBox::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(proxy);
    }
}

void TaskScaledParameters::apply()
{
    FCMD_OBJ_CMD(TransformedView->getObject(),"Factor = " << getFactor());
    ui->spinOccurrences->apply();
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgScaledParameters::TaskDlgScaledParameters(ViewProviderScaled *ScaledView)
    : TaskDlgTransformedParameters(ScaledView, new TaskScaledParameters(ScaledView))
{
}
//==== calls from the TaskView ===============================================================

bool TaskDlgScaledParameters::accept()
{

        parameter->apply();

    return TaskDlgTransformedParameters::accept();
}

#include "moc_TaskScaledParameters.cpp"
