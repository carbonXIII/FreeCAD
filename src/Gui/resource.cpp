/***************************************************************************
 *   Copyright (c) 2004 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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


#include "PreCompiled.h"

#include "BitmapFactory.h"
#include "WidgetFactory.h"
#include "Workbench.h"

// INCLUDE YOUR PREFERENCE PAGES HERE
//
#include "DlgPreferencesImp.h"
#include "DlgSettings3DViewImp.h"
#include "DlgSettingsUI.h"
#include "DlgSettingsNavigation.h"
#include "DlgSettingsSelection.h"
#include "DlgSettingsViewColor.h"
#include "DlgSettingsDrawStyles.h"
#include "DlgGeneralImp.h"
#include "DlgEditorImp.h"
#include "DlgSettingsNotificationArea.h"
#include "DlgSettingsPythonConsole.h"
#include "DlgSettingsMacroImp.h"
#include "DlgSettingsDocumentImp.h"
#include "DlgSettingsObjects.h"
#include "DlgReportViewImp.h"
#include "DlgSettingsWorkbenchesImp.h"
#include "DlgSettingsPreferencePack.h"

#include "DlgToolbarsImp.h"
#include "DlgActionsImp.h"
#include "DlgKeyboardImp.h"
#include "DlgCustomizeSpaceball.h"
#include "DlgCustomizeSpNavSettings.h"
#include "DlgIconBrowser.h"
#include "DlgSettingsCacheDirectory.h"
#include "InputField.h"
#include "QuantitySpinBox.h"
#include "PrefWidgets.h"

using namespace Gui;
using namespace Gui::Dialog;

/**
 * Registers all preference pages or widgets to create them dynamically at any later time.
 */
WidgetFactorySupplier::WidgetFactorySupplier()
{
    // ADD YOUR PREFERENCE PAGES HERE
    //
    //
    new PrefPageProducer<DlgGeneralImp>            ( QT_TRANSLATE_NOOP("QObject","General") );
    DlgGeneralImp::attachObserver();
    new PrefPageProducer<DlgSettingsDocumentImp>   ( QT_TRANSLATE_NOOP("QObject","General") );
    new PrefPageProducer<DlgSettingsObjects>       ( QT_TRANSLATE_NOOP("QObject","General") );
    new PrefPageProducer<DlgSettingsSelection>     ( QT_TRANSLATE_NOOP("QObject","General") );
    new PrefPageProducer<DlgSettingsCacheDirectory>( QT_TRANSLATE_NOOP("QObject","General") );
    new PrefPageProducer<DlgSettingsNotificationArea> ( QT_TRANSLATE_NOOP("QObject","General") );
    new PrefPageProducer<DlgReportViewImp>         ( QT_TRANSLATE_NOOP("QObject","General") );
    new PrefPageProducer<DlgSettingsPreferencePack>( QT_TRANSLATE_NOOP("QObject","General") );
    new PrefPageProducer<DlgSettings3DViewImp>     ( QT_TRANSLATE_NOOP("QObject","Display") );
    DlgSettings3DViewImp::attachObserver();
    new PrefPageProducer<DlgSettingsUI>            ( QT_TRANSLATE_NOOP("QObject","Display") );
    new PrefPageProducer<DlgSettingsNavigation>    ( QT_TRANSLATE_NOOP("QObject","Display") );
    new PrefPageProducer<DlgSettingsViewColor>     ( QT_TRANSLATE_NOOP("QObject","Display") );
    new PrefPageProducer<DlgSettingsDrawStyles>    ( QT_TRANSLATE_NOOP("QObject","Display") );
    new PrefPageProducer<DlgSettingsMacroImp>         ( QT_TRANSLATE_NOOP("QObject", "Python"));
    new PrefPageProducer<DlgSettingsPythonConsole>    ( QT_TRANSLATE_NOOP("QObject", "Python"));
    new PrefPageProducer<DlgSettingsEditorImp>        ( QT_TRANSLATE_NOOP("QObject", "Python"));

    // ADD YOUR CUSTOMIZE PAGES HERE
    //
    //
    new CustomPageProducer<DlgCustomKeyboardImp>;
    new CustomPageProducer<DlgCustomToolbarsImp>;
    new CustomPageProducer<DlgCustomActionsImp>;
    new CustomPageProducer<DlgIconBrowser>;
    new CustomPageProducer<DlgCustomizeSpNavSettings>;
    new CustomPageProducer<DlgCustomizeSpaceball>;

    // ADD YOUR PREFERENCE WIDGETS HERE
    //
    //
    new WidgetProducer<Gui::InputField>;
    new WidgetProducer<Gui::PrefSpinBox>;
    new WidgetProducer<Gui::PrefDoubleSpinBox>;
    new WidgetProducer<Gui::PrefLineEdit>;
    new WidgetProducer<Gui::PrefTextEdit>;
    new WidgetProducer<Gui::PrefComboBox>;
    new WidgetProducer<Gui::PrefFontBox>;
    new WidgetProducer<Gui::PrefCheckBox>;
    new WidgetProducer<Gui::PrefRadioButton>;
    new WidgetProducer<Gui::PrefSlider>;
    new WidgetProducer<Gui::PrefFileChooser>;
    new WidgetProducer<Gui::PrefColorButton>;
    new WidgetProducer<Gui::PrefUnitSpinBox>;
    new WidgetProducer<Gui::PrefQuantitySpinBox>;
    new WidgetProducer<Gui::CommandIconView>;
    new WidgetProducer<Gui::AccelLineEdit>;
    new WidgetProducer<Gui::ActionSelector>;
    new WidgetProducer<Gui::ColorButton>;
    new WidgetProducer<Gui::UrlLabel>;
    new WidgetProducer<Gui::StatefulLabel>;
    new WidgetProducer<Gui::FileChooser>;
    new WidgetProducer<Gui::UIntSpinBox>;
    new WidgetProducer<Gui::IntSpinBox>;
    new WidgetProducer<Gui::DoubleSpinBox>;
    new WidgetProducer<Gui::QuantitySpinBox>;
}
