# -*- coding: utf-8 -*-
# ***************************************************************************
# *   Copyright (c) 2022 Zheng Lei (realthunder) <realthunder.dev@gmail.com>*
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU Lesser General Public License (LGPL)    *
# *   as published by the Free Software Foundation; either version 2 of     *
# *   the License, or (at your option) any later version.                   *
# *   for detail see the LICENCE text file.                                 *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU Library General Public License for more details.                  *
# *                                                                         *
# *   You should have received a copy of the GNU Library General Public     *
# *   License along with this program; if not, write to the Free Software   *
# *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
# *   USA                                                                   *
# *                                                                         *
# ***************************************************************************
'''Utilites for generating C++ code for parameters management using Python Cog
'''
import cog
import inspect
from os import path

def quote(txt, indent=0):
    lines = [ ' '*indent + '"' + l.replace('"', '\"').replace('\\', '\\\\') for l in txt.split('\n')]
    return '\\n"\n'.join(lines) + '"'

def init_params(params, namespace, class_name, param_path, header_file=None):
    for param in params:
        param.path = param_path
        if not header_file:
            header_file = [f'{namespace}/{class_name}.h']
        param.header_file = header_file + getattr(param.proxy, 'header_file', [])
        param.namespace = namespace
        param.class_name = class_name
    return params

def auto_comment(frame=1, msg=None, count=1):
    trace = []
    for stack in inspect.stack()[frame:frame+count]:
        filename = path.normpath(stack[1]).split('/src/')[-1]
        if filename.find('<') >= 0:
            break
        lineno = stack[2]
        trace.insert(0, f'{filename}:{lineno}')
    return f'{"// Auto generated code" if msg is None else msg} ({" <- ".join(trace)})'

def trace_comment():
    return auto_comment(2)

def declare_begin(module, header=True):
    class_name = module.ClassName
    namespace = module.NameSpace
    params = module.Params
    param_path = module.ParamPath
    param_file = getattr(module, 'ParamSource', f'{namespace}/{class_name}.py')
    header_file = getattr(module, 'HeaderFile', f'{namespace}/{class_name}.h')
    source_file = getattr(module, 'SourceFile', f'{namespace}/{class_name}.cpp')
    class_doc = module.ClassDoc
    signal = getattr(module, 'Signal', False)

    if header:
        cog.out(f'''
{trace_comment()}
#include <Base/Parameter.h>
{"#include <boost_signals2.hpp>" if signal else ""}
''')

    cog.out(f'''
{trace_comment()}
namespace {namespace} {{
/** {class_doc}

 * The parameters are under group "{param_path}"
 *
 * This class is auto generated by {param_file}. Modify that file
 * instead of this one, if you want to add any parameter. You need
 * to install Cog Python package for code generation:
 * @code
 *     pip install cogapp
 * @endcode
 *
 * Once modified, you can regenerate the header and the source file,
 * @code
 *     python3 -m cogapp -r {header_file} {source_file}
 * @endcode
 *
 * You can add a new parameter by adding lines in {param_file}. Available
 * parameter types are 'Int, UInt, String, Bool, Float'. For example, to add
 * a new Int type parameter,
 * @code
 *     ParamInt(parameter_name, default_value, documentation, on_change=False)
 * @endcode
 *
 * If there is special handling on parameter change, pass in on_change=True.
 * And you need to provide a function implementation in {source_file} with
 * the following signature.
 * @code
 *     void {class_name}:on<parameter_name>Changed()
 * @endcode
 */
class {namespace}Export {class_name} {{
public:
    static ParameterGrp::handle getHandle();
''')
    if signal:
        cog.out(f'''
    static boost::signals2::signal<void (const char*)> &signalParamChanged();
    static void signalAll();
''')

    for param in params:
        cog.out(f'''
    {trace_comment()}
    //@{{
    /// Accessor for parameter {param.name}''')
        if param._doc:
            cog.out(f'''
    ///''')
            for line in param._doc.split('\n'):
                cog.out(f'''
    /// {line}''')
        cog.out(f'''
    static const {param.C_Type} & get{param.name}();
    static const {param.C_Type} & default{param.name}();
    static void remove{param.name}();
    static void set{param.name}(const {param.C_Type} &v);
    static const char *doc{param.name}();''')
        if param.on_change:
            cog.out(f'''
    static void on{param.name}Changed();''')
        cog.out(f'''
    //@}}
''')


def declare_end(module):
    class_name = module.ClassName
    namespace = module.NameSpace

    cog.out(f'''
{trace_comment()}
}}; // class {class_name}
}} // namespace {namespace}
''')


def define(module, header=True):
    class_name = module.ClassName
    namespace = module.NameSpace
    params = module.Params
    param_path = module.ParamPath
    class_doc = module.ClassDoc
    signal = getattr(module, 'Signal', False)

    if header:
        cog.out(f'''
{trace_comment()}
#include <unordered_map>
#include <App/Application.h>
#include <App/DynamicProperty.h>
#include "{class_name}.h"
using namespace {namespace};
''')

    cog.out(f'''
{trace_comment()}
namespace {{
class {class_name}P: public ParameterGrp::ObserverType {{
public:
    ParameterGrp::handle handle;
    std::unordered_map<const char *,void(*)({class_name}P*),App::CStringHasher,App::CStringHasher> funcs;
''')

    if signal:
        cog.out(f'''
    {trace_comment()}
    boost::signals2::signal<void (const char*)> signalParamChanged;
    void signalAll()
    {{''')
        for param in params:
            cog.out(f'''
        signalParamChanged("{param.name}");''')
        cog.out(f'''

    {trace_comment()}
    }}''')

    for param in params:
        cog.out(f'''
    {param.C_Type} {param.name};''')

    cog.out(f'''

    {trace_comment()}
    {class_name}P() {{
        handle = App::GetApplication().GetParameterGroupByPath("{param_path}");
        handle->Attach(this);
''')

    for param in params:
        cog.out(f'''
        {param.name} = {param.getter('handle')};
        funcs["{param.name}"] = &{class_name}P::update{param.name};''')

    cog.out(f'''
    }}

    {trace_comment()}
    ~{class_name}P() {{
    }}
''')
    cog.out(f'''
    {trace_comment()}
    void OnChange(Base::Subject<const char*> &, const char* sReason) {{
        if(!sReason)
            return;
        auto it = funcs.find(sReason);
        if(it == funcs.end())
            return;
        it->second(this);
        {"signalParamChanged(sReason);" if signal else ""}
    }}

''')

    for param in params:
        if not param.on_change:
            cog.out(f'''
    {trace_comment()}
    static void update{param.name}({class_name}P *self) {{
        self->{param.name} = {param.getter('self->handle')};
    }}''')
        else:
            cog.out(f'''
    {trace_comment()}
    static void update{param.name}({class_name}P *self) {{
        auto v = {param.getter('self->handle')};
        if (self->{param.name} != v) {{
            self->{param.name} = v;
            {class_name}::on{param.name}Changed();
        }}
    }}''')

    cog.out(f'''
}};

{trace_comment()}
{class_name}P *instance() {{
    static {class_name}P *inst = new {class_name}P;
    return inst;
}}

}} // Anonymous namespace
''')
    cog.out(f'''
{trace_comment()}
ParameterGrp::handle {class_name}::getHandle() {{
    return instance()->handle;
}}
''')

    if signal:
        cog.out(f'''
{trace_comment()}
boost::signals2::signal<void (const char*)> &
{class_name}::signalParamChanged() {{
    return instance()->signalParamChanged;
}}
''')
        cog.out(f'''
{trace_comment()}
void signalAll() {{
    instance()->signalAll();
}}
''')

    for param in params:
        cog.out(f'''
{trace_comment()}
const char *{class_name}::doc{param.name}() {{
    return {param.doc(class_name)};
}}
''')
        cog.out(f'''
{trace_comment()}
const {param.C_Type} & {class_name}::get{param.name}() {{
    return instance()->{param.name};
}}
''')
        cog.out(f'''
{trace_comment()}
const {param.C_Type} & {class_name}::default{param.name}() {{
    const static {param.C_Type} def = {param.default};
    return def;
}}
''')
        cog.out(f'''
{trace_comment()}
void {class_name}::set{param.name}(const {param.C_Type} &v) {{
    {param.setter()};
    instance()->{param.name} = v;
}}
''')
        cog.out(f'''
{trace_comment()}
void {class_name}::remove{param.name}() {{
    instance()->handle->Remove{param.Type}("{param.name}");
}}
''')

def widgets_declare(param_set):
    param_group = param_set.ParamGroup

    for title,params in param_group:
        name = title.replace(' ', '')
        cog.out(f'''

    {trace_comment()}
    QGroupBox * group{name} = nullptr;''')
        for param in params:
            param.declare_widget()

def widgets_init(param_set):
    param_group = param_set.ParamGroup

    cog.out(f'''
    auto layout = new QVBoxLayout(this);''')
    for title, params in param_group:
        name = title.replace(' ', '')
        cog.out(f'''


    {trace_comment()}
    group{name} = new QGroupBox(this);
    layout->addWidget(group{name});
    auto layoutHoriz{name} = new QHBoxLayout(group{name});
    auto layout{name} = new QGridLayout();
    layoutHoriz{name}->addLayout(layout{name});
    layoutHoriz{name}->addStretch();''')

        for row,param in enumerate(params):
            cog.out(f'''

    {trace_comment()}''')

            param.init_widget(row, name)

            cog.out(f'''
    {param.widget_name}->setEntryName("{param.name}");''')
            prefix = 'User parameter:BaseApp/Preferences/'
            if param.path.startswith(prefix):
                cog.out(f'''
    {param.widget_name}->setParamGrpPath("{param.path[len(prefix):]}");''')
            else:
                cog.out(f'''
    {param.widget_name}->setParamGrpPath("{param.path}");''')

    cog.out('''
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Expanding));
    retranslateUi();''')

def widgets_restore(param_set):
    param_group = param_set.ParamGroup

    cog.out(f'''
    {trace_comment()}''')
    for _,params in param_group:
        for param in params:
            cog.out(f'''
    {param.widget_name}->onRestore();''')

def widgets_save(param_set):
    param_group = param_set.ParamGroup

    cog.out(f'''
    {trace_comment()}''')
    for _,params in param_group:
        for param in params:
            cog.out(f'''
    {param.widget_name}->onSave();''')

def preference_dialog_declare_begin(param_set, header=True):
    namespace = param_set.NameSpace
    class_name = param_set.ClassName
    dialog_namespace = getattr(param_set, 'DialogNameSpace', 'Dialog')
    param_group = param_set.ParamGroup
    param_file = getattr(param_set, 'ParamSource', class_name + '.py')
    header_file = getattr(param_set, 'HeaderFile', class_name + '.h')
    source_file = getattr(param_set, 'SourceFile', class_name + '.cpp')
    class_doc = param_set.ClassDoc

    if header:
        cog.out(f'''
{trace_comment()}
#include <Gui/PropertyPage.h>
#include <Gui/PrefWidgets.h>''')

    cog.out(f'''
{trace_comment()}
class QLabel;
class QGroupBox;

namespace {namespace} {{
namespace {dialog_namespace} {{
/** {class_doc}

 * This class is auto generated by {param_file}. Modify that file
 * instead of this one, if you want to make any change. You need
 * to install Cog Python package for code generation:
 * @code
 *     pip install cogapp
 * @endcode
 *
 * Once modified, you can regenerate the header and the source file,
 * @code
 *     python3 -m cogapp -r {header_file} {source_file}
 * @endcode
 */
class {class_name} : public Gui::Dialog::PreferencePage
{{
    Q_OBJECT

public:
    {class_name}( QWidget* parent = 0 );
    ~{class_name}();

    void saveSettings();
    void loadSettings();
    void retranslateUi();

protected:
    void changeEvent(QEvent *e);

private:''')
    widgets_declare(param_set)


def preference_dialog_declare_end(param_set):
    class_name = param_set.ClassName
    namespace = param_set.NameSpace
    dialog_namespace = getattr(param_set, 'DialogNameSpace', 'Dialog')

    cog.out(f'''
{trace_comment()}
}};
}} // namespace {dialog_namespace}
}} // namespace {namespace}
''')

def preference_dialog_declare(param_set, header=True):
    preference_dialog_declare_begin(param_set, header)
    preference_dialog_declare_end(param_set)

def preference_dialog_define(param_set, header=True):
    param_group = param_set.ParamGroup
    class_name = param_set.ClassName
    dialog_namespace = getattr(param_set, 'DialogNameSpace', 'Dialog')
    namespace = f'{param_set.NameSpace}::{dialog_namespace}'
    param_file = getattr(param_set, 'ParamSource', class_name + '.py')
    header_file = getattr(param_set, 'HeaderFile', class_name + '.h')
    source_file = getattr(param_set, 'SourceFile', class_name + '.cpp')
    user_init = getattr(param_set, 'UserInit', '')
    headers = set()

    if header:
        cog.out(f'''
{trace_comment()}
#ifndef _PreComp_
#   include <QApplication>
#   include <QLabel>
#   include <QGroupBox>
#   include <QGridLayout>
#   include <QVBoxLayout>
#   include <QHBoxLayout>
#endif''')
        for _,params in param_group:
            for param in params:
                for header in param.header_file:
                    if header not in headers:
                        headers.add(header)
                        cog.out(f'''
#include <{header}>''')

    cog.out(f'''
{trace_comment()}
#include "{header_file}"
using namespace {namespace};
/* TRANSLATOR {namespace}::{class_name} */
''')

    cog.out(f'''
{trace_comment()}
{class_name}::{class_name}(QWidget* parent)
    : PreferencePage( parent )
{{
''')
    widgets_init(param_set)
    cog.out(f'''
    {trace_comment()}
    {user_init}
}}
''')
    cog.out(f'''
{trace_comment()}
{class_name}::~{class_name}()
{{
}}
''')
    cog.out(f'''
{trace_comment()}
void {class_name}::saveSettings()
{{''')
    widgets_save(param_set)
    cog.out(f'''
}}

{trace_comment()}
void {class_name}::loadSettings()
{{''')
    widgets_restore(param_set)
    cog.out(f'''
}}

{trace_comment()}
void {class_name}::retranslateUi()
{{
    setWindowTitle(QObject::tr("{param_set.Title}"));''')
    for title, params in param_group:
        name = title.replace(' ', '')
        cog.out(f'''
    group{name}->setTitle(QObject::tr("{title}"));''')
        for row,param in enumerate(params):
            param.retranslate()
    cog.out(f'''
}}

{trace_comment()}
void {class_name}::changeEvent(QEvent *e)
{{
    if (e->type() == QEvent::LanguageChange) {{
        retranslateUi();
    }}
    QWidget::changeEvent(e);
}}
''')

    cog.out(f'''
{trace_comment()}
#include "moc_{class_name}.cpp"
''')

class Param:
    WidgetPrefix = ''

    def __init__(self, name, default, doc='', title='', on_change=False, proxy=None, **kwd):
        self.name = name
        self.title = title if title else name
        self._default = default
        self._doc = doc
        self.on_change = on_change
        self.proxy = proxy

    def _declare_label(self):
        cog.out(f'''
    QLabel *label{self.name} = nullptr;''')

    def declare_label(self):
        if self.proxy:
            self.proxy.declare_label(self)
        else:
            self._declare_label()

    def _init_label(self, row, group_name):
        cog.out(f'''
    label{self.name} = new QLabel(this);
    layout{group_name}->addWidget(label{self.name}, {row}, 0);''')

    def init_label(self, row, group_name):
        if self.proxy:
            self.proxy.init_label(self, row, group_name)
        else:
            self._init_label(row, group_name)

    def _declare_widget(self):
        self.declare_label()
        cog.out(f'''
    {self.widget_type} *{self.widget_name} = nullptr;''')

    def declare_widget(self):
        if self.proxy:
            self.proxy.declare_widget(self)
        else:
            self._declare_widget()

    def _init_widget(self, row, group_name):
        self.init_label(row, group_name)
        cog.out(f'''
    {self.widget_name} = new {self.widget_type}(this);
    layout{group_name}->addWidget({self.widget_name}, {row}, {self.widget_column});''')
        if self.widget_setter:
            cog.out(f'''
    {self.widget_name}->{self.widget_setter}({self.namespace}::{self.class_name}::default{self.name}());''')

    def init_widget(self, row, group_name):
        if self.proxy:
            self.proxy.init_widget(self, row, group_name)
        else:
            self._init_widget(row, group_name)

    def _retranslate_label(self):
        cog.out(f'''
    label{self.name}->setText(QObject::tr("{self.title}"));
    label{self.name}->setToolTip({self.widget_name}->toolTip());''')

    def retranslate_label(self):
        if self.proxy:
            self.proxy.retranslate_label(self)
        else:
            self._retranslate_label()

    def _retranslate(self):
        cog.out(f'''
    {self.widget_name}->setToolTip(QApplication::translate("{self.class_name}", {self.namespace}::{self.class_name}::doc{self.name}()));''')
        self.retranslate_label()

    def retranslate(self):
        if self.proxy:
            self.proxy.retranslate(self)
        else:
            self._retranslate()

    @property
    def default(self):
        return self._default

    def doc(self, class_name):
        if not self._doc:
            return '""'
        return f'''QT_TRANSLATE_NOOP("{class_name}",
{quote(self._doc)})'''

    @property
    def widget_type(self):
        if self.proxy:
            return self.proxy.widget_type(self)
        return self.WidgetType

    @property
    def widget_prefix(self):
        if self.proxy:
            return self.proxy.widget_prefix(self)
        return self.WidgetPrefix

    @property
    def widget_setter(self):
        if self.proxy:
            return self.proxy.widget_setter(self)
        return self.WidgetSetter

    @property
    def widget_name(self):
        return f'{self.widget_prefix}{self.name}'

    @property
    def widget_column(self):
        return 1

    def getter(self, handle):
        return f'{handle}->Get{self.Type}("{self.name}", {self.default})'

    def setter(self):
        return f'instance()->handle->Set{self.Type}("{self.name}",v)'


class ParamBool(Param):
    Type = 'Bool'
    C_Type = 'bool'
    WidgetType = 'Gui::PrefCheckBox'
    WidgetSetter = 'setChecked'

    @property
    def default(self):
        if isinstance(self._default, str):
            return self._default
        return 'true' if self._default else 'false'

    def _declare_label(self):
        pass

    def _init_label(self, _row, _group_name):
        pass

    @property
    def widget_column(self):
        return 0

    def _retranslate_label(self):
        cog.out(f'''
    {self.widget_name}->setText(QObject::tr("{self.title}"));''')

class ParamFloat(Param):
    Type = 'Float'
    C_Type = 'double'
    WidgetType = 'Gui::PrefDoubleSpinBox'
    WidgetSetter = 'setValue'

class ParamString(Param):
    Type = 'ASCII'
    C_Type = 'std::string'
    WidgetType = 'Gui::PrefLineEdit'
    WidgetSetter = 'setText'

    @property
    def default(self):
        return f'"{self._default}"'

class ParamQString(Param):
    Type = 'ASCII'
    C_Type = 'QString'
    WidgetType = 'Gui::PrefLineEdit'
    WidgetSetter = 'setText'

    @property
    def default(self):
        return f'QStringLiteral("{self._default}")'

    def getter(self, handle):
        return f'QString::fromUtf8({handle}->Get{self.Type}("{self.name}", "{self._default}").c_str())'

    def setter(self):
        return f'instance()->handle->Set{self.Type}("{self.name}",v.toUtf8().constData())'

class ParamInt(Param):
    Type = 'Int'
    C_Type = 'long'
    WidgetType = 'Gui::PrefSpinBox'
    WidgetSetter = 'setValue'

class ParamUInt(Param):
    Type = 'Unsigned'
    C_Type = 'unsigned long'
    WidgetType = 'Gui::PrefSpinBox'
    WidgetSetter = 'setValue'

class ParamHex(ParamUInt):
    @property
    def default(self):
        return '0x%08X' % self._default

class ParamProxy:
    WidgetType = None
    WidgetPrefix = ''
    WidgetSetter = None

    def __init__(self, param_bool=None):
        self.param_bool = param_bool

    def declare_label(self, param):
        if not self.param_bool:
            param._declare_label()

    def widget_prefix(self, param):
        return self.WidgetPrefix if self.WidgetPrefix else param.WidgetPrefix

    def widget_type(self, param):
        return self.WidgetType if self.WidgetType else param.WidgetType

    def widget_setter(self, param):
        return self.WidgetSetter if self.WidgetSetter else param.WidgetSetter

    def declare_widget(self, param):
        if self.param_bool:
            self.param_bool.declare_widget()
        param._declare_widget()

    def init_label(self, param, row, group_name):
        if not self.param_bool:
            param._init_label(row, group_name)

    def init_widget(self, param, row, group_name):
        param._init_widget(row, group_name)
        if self.param_bool:
            self.param_bool.init_widget(row, group_name)
            cog.out(f'''
    {param.widget_name}->setEnabled({self.param_bool.widget_name}->isChecked());
    connect({self.param_bool.widget_name}, SIGNAL(toggled(bool)), {param.widget_name}, SLOT(setEnabled(bool)));''')

    def retranslate_label(self, param):
        if not self.param_bool:
            param._retranslate_label()

    def retranslate(self, param):
        param._retranslate()
        if self.param_bool:
            self.param_bool.retranslate()

class ComboBoxItem:
    def __init__(self, text, tooltips=None, data=None):
        self.text = text
        self.tooltips = tooltips
        self._data = data

    @property
    def data(self):
        if self._data is None:
            return 'QVariant()'
        if isinstance(self._data, str):
            return f'QByteArray("{self._data}")'
        return self._data

class ParamComboBox(ParamProxy):
    WidgetType = 'Gui::PrefComboBox'

    def __init__(self, items, translate=True, param_bool=None):
        super().__init__(param_bool)
        self.translate = translate
        self.items = []
        for item in items:
            if isinstance(item, str):
                item = ComboBoxItem(item);
            elif isinstance(item, tuple):
                item = ComboBoxItem(*item)
            else:
                assert(isinstance(item, ComboBoxItem))
            self.items.append(item)

    def widget_setter(self, _param):
        return None

    def init_widget(self, param, row, group_name):
        super().init_widget(param, row, group_name)
        if self.translate:
            cog.out(f'''
    for (int i=0; i<{len(self.items)}; ++i) {trace_comment()}
        {param.widget_name}->addItem(QString());''')

        for i,item in enumerate(self.items):
            if not self.translate:
                cog.out(f'''
    {param.widget_name}->addItem(QStringLiteral("{item.text}"));''')
            if item._data is not None:
                cog.out(f'''
    {param.widget_name}->setItemData({param.widget_name}->count()-1, {item.data});''')

        cog.out(f'''
    {param.widget_name}->setCurrentIndex({param.namespace}::{param.class_name}::default{param.name}());''')

    def retranslate(self, param):
        super().retranslate(param)
        cog.out(f'''
    {trace_comment()}''')
        for i,item in enumerate(self.items):
            if self.translate:
                cog.out(f'''
    {param.widget_name}->setItemText({i}, QObject::tr("{item.text}"));''')
            if item.tooltips:
                cog.out(f'''
    {param.widget_name}->setItemData({i}, QObject::tr("{item.tooltips}"), Qt::ToolTipRole);''')

class ParamLinePattern(ParamProxy):
    WidgetType = 'Gui::PrefLinePattern'

    def widget_setter(self, _param):
        return None

    def init_widget(self, param, row, group_name):
        super().init_widget(param, row, group_name)
        cog.out(f'''
    {trace_comment()}
    for (int i=1; i<{param.widget_name}->count(); ++i) {{
        if ({param.widget_name}->itemData(i).toInt() == {param.default})
            {param.widget_name}->setCurrentIndex(i);
    }}''')

class ParamColor(ParamProxy):
    WidgetType = 'Gui::PrefColorButton'
    WidgetSetter = 'setPackedColor'

    def __init__(self, param_bool=None, transparency=True):
        super().__init__(param_bool)
        self.transparency = transparency

    def init_widget(self, param, row, group_name):
        super().init_widget(param, row, group_name)
        if self.transparency:
            cog.out(f'''
    {param.widget_name}->setAllowTransparency(true);''')

class ParamFile(ParamProxy):
    WidgetType = 'Gui::PrefFileChooser'
    WidgetSetter = 'setFileNameStd'

class ParamSpinBox(ParamProxy):
    def __init__(self, value_min, value_max, value_step, param_bool=None):
        super().__init__(param_bool)
        self.value_min = value_min
        self.value_max = value_max
        self.value_step = value_step

    def init_widget(self, param, row, group_name):
        super().init_widget(param, row, group_name)
        cog.out(f'''
    {trace_comment()}
    {param.widget_name}->setMaximum({self.value_min});
    {param.widget_name}->setMaximum({self.value_max});
    {param.widget_name}->setSingleStep({self.value_step});''')

class ParamShortcutEdit(ParamProxy):
    WidgetType = 'Gui::PrefAccelLineEdit'
    WidgetSetter = 'setDisplayText'
