/***************************************************************************
 *   Copyright (c) 2005 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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
#include <boost/signals2/connection.hpp>
#ifndef _PreComp_
# include <QAction>
# include <QApplication>
# include <QToolBar>
# include <QToolButton>
# include <QMenu>
# include <QHBoxLayout>
# include <QMouseEvent>
# include <QCheckBox>
# include <QPointer>
# include <QPainter>
# include <QStatusBar>
# include <QMenuBar>
#endif

#include <QWidgetAction>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

#include <Base/Tools.h>
#include <Base/Console.h>

#include "ToolBarManager.h"

#include "Action.h"
#include "Application.h"
#include "Command.h"
#include "MainWindow.h"
#include "OverlayWidgets.h"

FC_LOG_LEVEL_INIT("ToolBar", true, 2)


using namespace Gui;

ToolBarItem::ToolBarItem() : visibility(HideStyle::VISIBLE)
{
}

ToolBarItem::ToolBarItem(ToolBarItem* item, HideStyle visibility) : visibility(visibility)
{
    if (item) {
        item->appendItem(this);
    }
}

ToolBarItem::~ToolBarItem()
{
    clear();
}

void ToolBarItem::setCommand(const std::string& name)
{
    _name = name;
}

const std::string & ToolBarItem::command() const
{
    return _name;
}

void ToolBarItem::setID(const QString& name)
{
    _id = name;
}

const QString & ToolBarItem::id() const
{
    return _id;
}

bool ToolBarItem::hasItems() const
{
    return _items.count() > 0;
}

ToolBarItem* ToolBarItem::findItem(const std::string& name)
{
    if ( _name == name ) {
        return this;
    }

    for (auto it : qAsConst(_items)) {
        if (it->_name == name) {
            return it;
        }
    }

    return nullptr;
}

ToolBarItem* ToolBarItem::copy() const
{
    auto root = new ToolBarItem;
    root->setCommand( command() );
    root->setID( id() );

    QList<ToolBarItem*> items = getItems();
    for (auto it : items) {
        root->appendItem(it->copy());
    }

    return root;
}

uint ToolBarItem::count() const
{
    return _items.count();
}

void ToolBarItem::appendItem(ToolBarItem* item)
{
    _items.push_back( item );
}

bool ToolBarItem::insertItem( ToolBarItem* before, ToolBarItem* item)
{
    int pos = _items.indexOf(before);
    if (pos != -1) {
        _items.insert(pos, item);
        return true;
    }

    return false;
}

void ToolBarItem::removeItem(ToolBarItem* item)
{
    int pos = _items.indexOf(item);
    if (pos != -1) {
        _items.removeAt(pos);
    }
}

void ToolBarItem::clear()
{
    for (auto it : qAsConst(_items)) {
        delete it;
    }

    _items.clear();
}

ToolBarItem& ToolBarItem::operator << (ToolBarItem* item)
{
    appendItem(item);
    return *this;
}

ToolBarItem& ToolBarItem::operator << (const std::string& command)
{
    auto item = new ToolBarItem(this);
    item->setCommand(command);
    return *this;
}

QList<ToolBarItem*> ToolBarItem::getItems() const
{
    return _items;
}

// -----------------------------------------------------------

ToolBarManager* ToolBarManager::_instance=nullptr;

ToolBarManager* ToolBarManager::getInstance()
{
    if ( !_instance )
        _instance = new ToolBarManager;
    return _instance;
}

void ToolBarManager::destruct()
{
    delete _instance;
    _instance = nullptr;
}

namespace Gui {

class ToolBarArea : public QWidget
{
    using inherited = QWidget;
public:
    ToolBarArea(QWidget *parent,
                ParameterGrp::handle hParam,
                boost::signals2::scoped_connection &conn,
                QTimer *timer = nullptr)
        : QWidget(parent)
        , _sizingTimer(timer)
        , _hParam(hParam)
        , _conn(conn)
    {
        _layout = new QHBoxLayout(this);
        _layout->setMargin(0);
    }

    void addWidget(QWidget *w)
    {
        if (_layout->indexOf(w) < 0) {
            _layout->addWidget(w);
            ToolBarManager::getInstance()->setToolBarMovable(qobject_cast<QToolBar*>(w));
            adjustParentSize();
            QString name = w->objectName();
            if (!name.isEmpty()) {
                Base::ConnectionBlocker block(_conn);
                _hParam->SetInt(w->objectName().toUtf8().constData(), _layout->count()-1);
            }
        }
    }

    void insertWidget(int idx, QWidget *w)
    {
        int index = _layout->indexOf(w);
        if (index == idx)
            return;
        if (index > 0)
            _layout->removeWidget(w);
        _layout->insertWidget(idx, w);
        ToolBarManager::getInstance()->setToolBarMovable(qobject_cast<QToolBar*>(w));
        adjustParentSize();
        saveState();
    }

    void adjustParentSize()
    {
        if (_sizingTimer) {
            _sizingTimer->start(10);
        }
    }

    void removeWidget(QWidget *w)
    {
        _layout->removeWidget(w);
        if (auto tb = qobject_cast<QToolBar*>(w)) {
            if (auto grip = tb->findChild<ToolBarGrip*>()) {
                tb->removeAction(grip->action());
                grip->deleteLater();
            }
        }
        adjustParentSize();
        QString name = w->objectName();
        if (!name.isEmpty()) {
            Base::ConnectionBlocker block(_conn);
            _hParam->RemoveInt(name.toUtf8().constData());
        }
    }

    QWidget *widgetAt(int index) const
    {
        auto item = _layout->itemAt(index);
        return item ? item->widget() : nullptr;
    }

    int count() const
    {
        return _layout->count();
    }

    int indexOf(QWidget *w) const
    {
        return _layout->indexOf(w);
    }

    template<class F>
    void foreachToolBar(F &&f)
    {
        for (int i = 0, c = _layout->count(); i < c; ++i) {
            auto toolbar = qobject_cast<QToolBar*>(widgetAt(i));
            if (!toolbar || toolbar->objectName().isEmpty()
                         || toolbar->objectName().startsWith(QStringLiteral("*")))
                continue;
            f(toolbar, i, this);
        }
    }

    void saveState()
    {
        Base::ConnectionBlocker block(_conn);
        for (auto &v : _hParam->GetIntMap()) {
            _hParam->RemoveInt(v.first.c_str());
        }
        foreachToolBar([this](QToolBar *toolbar, int idx, ToolBarArea*) {
            _hParam->SetInt(toolbar->objectName().toUtf8().constData(), idx);
        });
    };

    void restoreState(const std::map<int, QToolBar*> &toolbars)
    {
        for (auto &v : toolbars) {
            bool vis = v.second->isVisible();
            getMainWindow()->removeToolBar(v.second);
            v.second->setOrientation(Qt::Horizontal);
            insertWidget(v.first, v.second);
            ToolBarManager::getInstance()->setToolBarVisible(v.second, vis);
        }

        for (auto &v : _hParam->GetBoolMap()) {
            auto w = findChild<QWidget*>(QString::fromUtf8(v.first.c_str()));
            if (w)
                w->setVisible(v.second);
        }
    };

private:
    QHBoxLayout *_layout;
    QPointer<QTimer> _sizingTimer;
    ParameterGrp::handle _hParam;
    boost::signals2::scoped_connection &_conn;
};

class ToolBar: public QToolBar
{
public:
    void initStyleOption(QStyleOptionToolBar *option) const
    {
        QToolBar::initStyleOption(option);
    }
};

} // namespace Gui


//////////////////////////////////////////////////////////////////////

ToolBarGrip::ToolBarGrip(QToolBar * parent)
    :QWidget(parent)
{
    QStyle *style = parent->style();
    QStyleOptionToolBar opt;
    static_cast<ToolBar*>(parent)->initStyleOption(&opt);
    opt.features = QStyleOptionToolBar::Movable;
    int width = style->subElementRect(QStyle::SE_ToolBarHandle, &opt, parent).width();
    this->setFixedWidth(width+4);

    setCursor(Qt::OpenHandCursor);
    setMouseTracking(true);
    auto actions = parent->actions();
    _action = parent->insertWidget(actions.isEmpty()?nullptr:actions[0], this);
    setVisible(true);
}

void ToolBarGrip::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (auto toolbar = qobject_cast<QToolBar*>(parentWidget())) {
        QStyle *style = toolbar->style();
        QStyleOptionToolBar opt;
        static_cast<ToolBar*>(toolbar)->initStyleOption(&opt);
        opt.features = QStyleOptionToolBar::Movable;
        // opt.rect = style->subElementRect(QStyle::SE_ToolBarHandle, &opt, toolbar);
        opt.rect = this->rect();
        style->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &painter, toolbar);
    }
    else {
        painter.setPen(Qt::transparent);
        painter.setOpacity(0.5);
        painter.setBrush(QBrush(Qt::black, Qt::Dense6Pattern));
        QRect rect(this->rect());
        painter.drawRect(rect);
    }
}

void ToolBarGrip::mouseMoveEvent(QMouseEvent *me)
{
    auto toolbar = qobject_cast<QToolBar*>(parentWidget());
    if (!toolbar) {
        return;
    }
    auto area = ToolBarManager::getInstance()->getToolBarArea(toolbar);
    if (!area)
        return;

    QPoint pos = me->globalPos();
    QRect rect(toolbar->mapToGlobal(QPoint(0,0)), toolbar->size());
    if (rect.contains(pos))
        return;

    // If mouse is out of the toolbar, remove the toolbar from area and float the
    // toolbar
    {
        QSignalBlocker blocker(toolbar);
        area->removeWidget(toolbar);
        ToolBarManager::getInstance()->addToolBarToMainWindow(toolbar);
        toolbar->setWindowFlags(Qt::Tool
                | Qt::FramelessWindowHint
                | Qt::X11BypassWindowManagerHint);
        toolbar->adjustSize();
        pos = toolbar->mapFromGlobal(pos);
        toolbar->move(pos.x()-10, pos.y()-10);
        ToolBarManager::getInstance()->setToolBarVisible(toolbar, true);
    }
    toolbar->topLevelChanged(true);

    // After removing from area, this grip will be deleted. In order to
    // continue toolbar dragging (because the mouse button is still pressed),
    // we fake mouse events and send to toolbar. For some reason,
    // send/postEvent() does not work, only timer works.
    QPointer tb(toolbar);
    QTimer::singleShot(0, [tb] {
        auto modifiers = QApplication::queryKeyboardModifiers();
        auto buttons = QApplication::mouseButtons();
        if (buttons != Qt::LeftButton
                || QWidget::mouseGrabber()
                || modifiers != Qt::NoModifier
                || !tb) {
            return;
        }
        QPoint pos(10, 10);
        QPoint globalPos(tb->mapToGlobal(pos));
        QMouseEvent mouseEvent(
                QEvent::MouseButtonPress,
                pos, globalPos, Qt::LeftButton, buttons, modifiers);
        QApplication::sendEvent(tb, &mouseEvent);

        // Mose follow the mouse press event with mouse move with some offset
        // in order to activate toolbar dragging.
        QPoint offset(30, 30);
        QMouseEvent mouseMoveEvent(
                QEvent::MouseMove,
                pos+offset, globalPos+offset,
                Qt::LeftButton, buttons, modifiers);
        QApplication::sendEvent(tb, &mouseMoveEvent);
    });
}

void ToolBarGrip::mousePressEvent(QMouseEvent *)
{
    setCursor(Qt::ClosedHandCursor);
}

void ToolBarGrip::mouseReleaseEvent(QMouseEvent *)
{
    setCursor(Qt::OpenHandCursor);
}

//////////////////////////////////////////////////////////////////////

ToolBarManager::ToolBarManager()
{
    hGeneral = App::GetApplication().GetUserParameter().GetGroup(
            "BaseApp/Preferences/General");

    hGlobal = App::GetApplication().GetUserParameter().GetGroup(
            "BaseApp/Workbench/Global/ToolBar");
    getGlobalToolBarNames();

    hStatusBar = App::GetApplication().GetUserParameter().GetGroup(
            "BaseApp/MainWindow/StatusBar");

    hMenuBarRight = App::GetApplication().GetUserParameter().GetGroup(
            "BaseApp/MainWindow/MenuBarRight");
    hMenuBarLeft = App::GetApplication().GetUserParameter().GetGroup(
            "BaseApp/MainWindow/MenuBarLeft");

    hPref = App::GetApplication().GetUserParameter().GetGroup(
            "BaseApp/MainWindow/ToolBars");

    hMovable = hPref->GetGroup("Movable");

    if (auto sb = getMainWindow()->statusBar()) {
        sb->installEventFilter(this);
        statusBarArea = new ToolBarArea(sb, hStatusBar, connParam);
        statusBarArea->setObjectName(QStringLiteral("StatusBarArea"));
        sb->insertPermanentWidget(2, statusBarArea);
        statusBarArea->show();
    }

    if (auto mb = getMainWindow()->menuBar()) {
        mb->installEventFilter(this);
        menuBarLeftArea = new ToolBarArea(mb, hMenuBarLeft, connParam, &menuBarTimer);
        menuBarLeftArea->setObjectName(QStringLiteral("MenuBarLeftArea"));
        mb->setCornerWidget(menuBarLeftArea, Qt::TopLeftCorner);
        menuBarLeftArea->show();
        menuBarRightArea = new ToolBarArea(mb, hMenuBarRight, connParam, &menuBarTimer);
        menuBarRightArea->setObjectName(QStringLiteral("MenuBarRightArea"));
        mb->setCornerWidget(menuBarRightArea, Qt::TopRightCorner);
        menuBarRightArea->show();
    }

    globalArea = defaultArea = Qt::TopToolBarArea;
    hMainWindow = App::GetApplication().GetUserParameter().GetGroup("BaseApp/Preferences/MainWindow");
    std::string defarea = hMainWindow->GetASCII("DefaultToolBarArea");
    if (defarea == "Bottom")
        defaultArea = Qt::BottomToolBarArea;
    else if (defarea == "Left")
        defaultArea = Qt::LeftToolBarArea;
    else if (defarea == "Right")
        defaultArea = Qt::RightToolBarArea;
    defarea = hMainWindow->GetASCII("GlobalToolBarArea");
    if (defarea == "Bottom")
        globalArea = Qt::BottomToolBarArea;
    else if (defarea == "Left")
        globalArea = Qt::LeftToolBarArea;
    else if (defarea == "Right")
        globalArea = Qt::RightToolBarArea;

    auto refreshParams = [this](const char *name) {
        if (!name || boost::equals(name, "ToolbarIconSize"))
            _toolBarIconSize = hGeneral->GetInt("ToolbarIconSize", 24);
        if (!name || boost::equals(name, "StatusBarIconSize"))
            _statusBarIconSize = hGeneral->GetInt("StatusBarIconSize", 0);
        if (!name || boost::equals(name, "MenuBarIconSize"))
            _menuBarIconSize = hGeneral->GetInt("MenuBarIconSize", 0);
        if (!name || boost::equals(name, "WorkbenchTabIconSize"))
            _workbenchTabIconSize = hGeneral->GetInt("WorkbenchTabIconSize", 0);
        if (!name || boost::equals(name, "WorkbenchComboIconSize"))
            _workbenchComboIconSize = hGeneral->GetInt("WorkbenchComboIconSize", 0);
    };
    refreshParams(nullptr);

    connParam = App::GetApplication().GetUserParameter().signalParamChanged.connect(
        [this, refreshParams](ParameterGrp *Param, ParameterGrp::ParamType, const char *Name, const char *) {
            if (Param == hGeneral && Name) {
                refreshParams(Name);
            }
            if (Param == hPref
                    || Param == hMovable
                    || Param == hStatusBar
                    || Param == hMenuBarRight
                    || Param == hMenuBarLeft
                    || (Param == hMainWindow
                        && Name
                        && boost::equals(Name, "DefaultToolBarArea"))) {
                timer.start(100);
            }
            else if (Param == hGlobal)
                getGlobalToolBarNames();
        });
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(onTimer()));

    timerChild.setSingleShot(true);
    QObject::connect(&timerChild, &QTimer::timeout, [this]() {
        toolBars();
    });

    menuBarTimer.setSingleShot(true);
    QObject::connect(&menuBarTimer, &QTimer::timeout, [this]() {
        if (auto menuBar = getMainWindow()->menuBar()) {
            menuBar->adjustSize();
        }
    });

    auto toolbars = toolBars();
    for (auto &v : hPref->GetBoolMap()) {
        if (v.first.empty())
            continue;
        QString name = QString::fromUtf8(v.first.c_str());
        QToolBar* toolbar = toolbars[name];
        if (!toolbar)
            toolbar = createToolBar(name);
        toolbar->toggleViewAction()->setVisible(false);
        setToolBarVisible(toolbar, false);
    }

    timerResize.setSingleShot(true);
    QObject::connect(&timerResize, &QTimer::timeout, [this]() {
        for (const auto &v : resizingToolBars) {
            if (v.second) {
                setToolBarIconSize(v.first);
            }
        }
        resizingToolBars.clear();
    });
}

ToolBarManager::~ToolBarManager()
{
}

int ToolBarManager::toolBarIconSize(QWidget *widget) const
{
    int s = _toolBarIconSize;
    if (widget) {
        if (qobject_cast<WorkbenchTabWidget*>(widget)) {
            if (_workbenchTabIconSize > 0)
                s = _workbenchTabIconSize;
            else
                s *= 0.8;
            widget = widget->parentWidget();
        }
        else if (qobject_cast<WorkbenchComboBox*>(widget)) {
            if (_workbenchComboIconSize > 0)
                s = _workbenchComboIconSize;
            else
                s *= 0.8;
            widget = widget->parentWidget();
        }
    }

    if (widget) {
        if (widget->parentWidget() == statusBarArea) {
            if (_statusBarIconSize > 0)
                s = _statusBarIconSize;
            else
                s *= 0.6;
        }
        else if (widget->parentWidget() == menuBarLeftArea
                || widget->parentWidget() == menuBarRightArea) {
            if (_menuBarIconSize > 0)
                s = _menuBarIconSize;
            else
                s *= 0.6;
        }
    }
    return std::max(s, 5);
}

void ToolBarManager::setToolBarIconSize(QToolBar *toolbar)
{
    int s = getInstance()->toolBarIconSize(toolbar);
    toolbar->setIconSize(QSize(s, s));
}

void ToolBarManager::getGlobalToolBarNames()
{
    globalToolBarNames.clear();
    globalToolBarNames.insert(QStringLiteral("File"));
    globalToolBarNames.insert(QStringLiteral("Structure"));
    globalToolBarNames.insert(QStringLiteral("Macro"));
    globalToolBarNames.insert(QStringLiteral("View"));
    globalToolBarNames.insert(QStringLiteral("Workbench"));
    for (auto &hGrp : this->hGlobal->GetGroups())
        globalToolBarNames.insert(QString::fromUtf8(hGrp->GetGroupName()));
}

bool ToolBarManager::isDefaultMovable() const
{
    return hMovable->GetBool("*", true);
}

void ToolBarManager::setDefaultMovable(bool enable)
{
    hMovable->Clear();
    hMovable->SetBool("*", enable);
}

struct ToolBarKey {
    Qt::ToolBarArea area;
    bool visible;
    int a;
    int b;

    ToolBarKey(QToolBar *tb)
        :area(getMainWindow()->toolBarArea(tb))
        ,visible(tb->isVisible())
    {
        int x = tb->x();
        int y = tb->y();
        switch(area) {
        case Qt::BottomToolBarArea:
            a = -y; b = x;
            break;
        case Qt::LeftToolBarArea:
            a = x; b = y;
            break;
        case Qt::RightToolBarArea:
            a = -x, b = y;
            break;
        default:
            a = y; b = x;
            break;
        }
    }

    bool operator<(const ToolBarKey &other) const {
        if (area < other.area)
            return true;
        if (area > other.area)
            return false;
        if (visible > other.visible)
            return true;
        if (visible < other.visible)
            return false;
        if (a < other.a)
            return true;
        if (a > other.a)
            return false;
        return b < other.b;
    }
};

static bool isToolBarAllowed(QToolBar *toolbar, Qt::ToolBarArea area)
{
    if (area == Qt::TopToolBarArea || area == Qt::BottomToolBarArea)
        return true;
    for (auto w : toolbar->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        if (qobject_cast<QToolButton*>(w)
                || qobject_cast<QMenu*>(w)
                || strstr(w->metaObject()->className(),"Separator"))
            continue;
        return false;
    }
    return true;
}

static bool isToolBarEmpty(QToolBar *toolbar)
{
    // Every QToolBar has a default tool button of private class type
    // QToolBarExtension, hence <= 1 here.
    return toolbar->findChildren<QWidget*>().size() <= 1;
}

void ToolBarManager::onTimer()
{
    std::string defarea = hMainWindow->GetASCII("DefaultToolBarArea");
    auto area = Qt::TopToolBarArea;
    if (defarea == "Bottom")
        area = Qt::BottomToolBarArea;
    else if (defarea == "Left")
        area = Qt::LeftToolBarArea;
    else if (defarea == "Right")
        area = Qt::RightToolBarArea;
    defarea = hMainWindow->GetASCII("GlobalToolBarArea");
    auto gArea = Qt::TopToolBarArea;
    if (defarea == "Bottom")
        gArea = Qt::BottomToolBarArea;
    else if (defarea == "Left")
        gArea = Qt::LeftToolBarArea;
    else if (defarea == "Right")
        gArea = Qt::RightToolBarArea;

    auto mw = getMainWindow();
    bool movable = isDefaultMovable();

    std::map<int, QToolBar*> sbToolBars;
    std::map<int, QToolBar*> mbRightToolBars;
    std::map<int, QToolBar*> mbLeftToolBars;
    std::multimap<ToolBarKey, QToolBar*> lines;
    for (auto &v : toolBars()) {
        if (!v.second)
            continue;
        QToolBar *tb = v.second;
        bool isGlobal = globalToolBarNames.count(v.first);
        auto defArea = isGlobal ? globalArea : defaultArea;
        auto curArea = isGlobal ? gArea : area;
        QByteArray name = v.first.toUtf8();
        int idx = hStatusBar->GetInt(name, -1);
        tb->setMovable(hMovable->GetBool(name, movable));

        if (idx >= 0) {
            sbToolBars[idx] = tb;
            continue;
        }

        idx = hMenuBarLeft->GetInt(name, -1);
        if (idx >= 0) {
            mbLeftToolBars[idx] = tb;
            continue;
        }

        idx = hMenuBarRight->GetInt(name, -1);
        if (idx >= 0) {
            mbRightToolBars[idx] = tb;
            continue;
        }

        if (tb->parentWidget() != getMainWindow()) {
            addToolBarToMainWindow(tb);
        }

        if (defArea != curArea && mw->toolBarArea(tb) == defArea)
            lines.emplace(ToolBarKey(tb),tb);
        if (tb->toggleViewAction()->isVisible())
            setToolBarVisible(tb, hPref->GetBool(name, tb->isVisible()));
    }

    bool first = true;
    int a;
    for (auto &v : lines) {
        auto toolbarArea = globalToolBarNames.count(
                v.second->objectName()) ? gArea : area;
        if (!isToolBarAllowed(v.second, toolbarArea))
            continue;
        Base::StateLocker guard(adding);
        if (first)
            first = false;
        else if (a != v.first.a)
            mw->addToolBarBreak(toolbarArea);
        a = v.first.a;
        mw->addToolBar(toolbarArea, v.second);
    }

    if (defaultArea != area || globalArea != gArea) {
        defaultArea = area;
        globalArea = gArea;
        mw->saveWindowSettings(true);
    }

    statusBarArea->restoreState(sbToolBars);
    menuBarRightArea->restoreState(mbRightToolBars);
    menuBarLeftArea->restoreState(mbLeftToolBars);
}

QToolBar *ToolBarManager::createToolBar(const QString &name)
{
    Base::StateLocker guard(adding);
    auto toolbar = new QToolBar(getMainWindow());
    getMainWindow()->addToolBar(
            globalToolBarNames.count(name) ? globalArea : defaultArea, toolbar);
    toolbar->setObjectName(name);
    connectToolBar(toolbar);
    return toolbar;
}

void ToolBarManager::connectToolBar(QToolBar *toolbar)
{
    auto &p = connectedToolBars[toolbar];
    if (p == toolbar)
        return;
    p = toolbar;
    toolbar->installEventFilter(this);
    connect(toolbar, SIGNAL(visibilityChanged(bool)), this, SLOT(onToggleToolBar(bool)));
    connect(toolbar, SIGNAL(movableChanged(bool)), this, SLOT(onMovableChanged(bool)));
    connect(toolbar, &QToolBar::topLevelChanged, this, [this, toolbar] {
        if (this->restored)
            getMainWindow()->saveWindowSettings(true);
        checkToolBarIconSize(toolbar);
    });
    connect(toolbar, &QToolBar::iconSizeChanged, this, [this, toolbar] {
        if (toolbar->parentWidget() == menuBarLeftArea) {
            menuBarLeftArea->adjustParentSize();
        }
        else if (toolbar->parentWidget() == menuBarRightArea) {
            menuBarRightArea->adjustParentSize();
        }
    });
    QByteArray name = p->objectName().toUtf8();
    p->setMovable(hMovable->GetBool(name, isDefaultMovable()));
    FC_TRACE("connect toolbar " << name.constData() << ' ' << p);
}

void ToolBarManager::removeToolBar(const QString &id)
{
    auto tb = getMainWindow()->findChild<QToolBar*>(id);
    if (tb) {
        getMainWindow()->removeToolBar(tb);
        delete tb;
        connectedToolBars.erase(tb);
        hPref->RemoveBool(id.toUtf8().constData());
    }
}

bool ToolBarManager::isCustomToolBarName(const char *name)
{
    int dummy;
    return boost::equals(name, "Custom") || sscanf(name, "Custom%d", &dummy)==1;
}

QString ToolBarManager::generateToolBarID(const char *groupName, const char *name)
{
    return QStringLiteral("Custom_%1_%2").arg(QString::fromUtf8(groupName),
                                              QString::fromUtf8(name));
}

void ToolBarManager::setup(ToolBarItem* toolBarItems)
{
    static QPointer<QWidget> _ActionWidget;
    if (!_ActionWidget) {
        _ActionWidget = new QWidget(getMainWindow());
        _ActionWidget->setObjectName(QStringLiteral("_fc_action_widget_"));
		// Although _ActionWidget has zero size, on MacOS it somehow has a
		// 'phantom' size without any visible content and will block the top
		// left tool buttons of the application main window. So we move it out
		// of the way.
		_ActionWidget->move(QPoint(-100,-100));

    } else {
        for (auto action : _ActionWidget->actions())
            _ActionWidget->removeAction(action);
    }

    this->toolbarNames.clear();

    std::map<int,int> widths;

    QList<ToolBarItem*> items = toolBarItems->getItems();
    auto toolbars = toolBars();
    for (auto item : items) {
        // search for the toolbar
        QString name;
        if (item->id().size())
            name = item->id();
        else
            name = QString::fromUtf8(item->command().c_str());

        this->toolbarNames << name;
        std::string toolbarName = item->command();
        bool visible = hPref->GetBool(toolbarName.c_str(), true);
        if (item->id().size()) {
            // Migrate to use toolbar ID instead of title for identification to
            // avoid name conflict when using custom toolbar
            bool v = hPref->GetBool(name.toUtf8().constData(), true);
            if (v != hPref->GetBool(name.toUtf8().constData(), false))
                visible = v;
        }

        QToolBar *toolbar = nullptr;
        auto it = toolbars.find(name);
        if (it != toolbars.end()) {
            toolbar = it->second;
            toolbars.erase(it);
        }
        bool newToolBar = false;
        if (!toolbar) {
            newToolBar = true;
            toolbar = createToolBar(name);
            QByteArray n(name.toUtf8());
            if (hPref->GetBool(n, true) != hPref->GetBool(n, false)) {
                // Make sure we remember the toolbar so that we can pre-create
                // it the next time the application is launched
                Base::ConnectionBlocker block(connParam);
                hPref->SetBool(n, true);
            }
        }

        bool toolbar_added = toolbar->windowTitle().isEmpty();

        // Mark view action as visible to bypass handling of toolbar's
        // ChildAdded event in eventFilter
        toolbar->toggleViewAction()->setVisible(true);

        // setup the toolbar
        setup(item, toolbar);
        if (isToolBarEmpty(toolbar)) {
            toolbar->toggleViewAction()->setVisible(false);
            FC_TRACE("Empty toolbar " << name.toUtf8().constData());
            continue;
        }

        toolbar->setWindowTitle(QApplication::translate("Workbench", toolbarName.c_str())); // i18n
        setToolBarVisible(toolbar, visible);

        auto actions = toolbar->actions();
        for (auto action : actions) {
            _ActionWidget->addAction(action);
        }

        // try to add some breaks to avoid to have all toolbars in one line
        if (toolbar_added) {
            auto area = getMainWindow()->toolBarArea(toolbar);
            if (!isToolBarAllowed(toolbar, area)) {
                addToolBarToMainWindow(toolbar);
            }

            int &top_width = widths[area];
            if (top_width > 0 && getMainWindow()->toolBarBreak(toolbar))
                top_width = 0;
            // the width() of a toolbar doesn't return useful results so we estimate
            // its size by the number of buttons and the icon size
            QList<QToolButton*> btns = toolbar->findChildren<QToolButton*>();
            top_width += (btns.size() * toolbar->iconSize().width());
            int max_width = toolbar->orientation() == Qt::Vertical
                ? getMainWindow()->height() : getMainWindow()->width();
            if (newToolBar && top_width > max_width) {
                top_width = 0;
                getMainWindow()->insertToolBarBreak(toolbar);
            }
        }
    }

    // hide all unneeded toolbars
    for (auto &v : toolbars) {
        QToolBar *toolbar = v.second;
        if (!toolbar)
            continue;
        // ignore toolbars which do not belong to the previously active workbench
        if (!toolbar->toggleViewAction()->isVisible())
            continue;
        toolbar->toggleViewAction()->setVisible(false);
        setToolBarVisible(toolbar, false);
    }
}

void ToolBarManager::setToolBarVisible(QToolBar *toolbar, bool show)
{
    QSignalBlocker blocker(toolbar);
    if (!show) {
        // make sure that the main window has the focus when hiding the toolbar with
        // the combo box inside
        QWidget *fw = QApplication::focusWidget();
        while (fw &&  !fw->isWindow()) {
            if (fw == toolbar) {
                getMainWindow()->setFocus();
                break;
            }
            fw = fw->parentWidget();
        }
    }
    toolbar->setVisible(show);
}

void ToolBarManager::setup(ToolBarItem* item, QToolBar* toolbar) const
{
    CommandManager& mgr = Application::Instance->commandManager();
    QList<ToolBarItem*> items = item->getItems();
    if (items.empty())
        return;
    QList<QAction*> actions = toolbar->actions();
    for (QList<ToolBarItem*>::ConstIterator it = items.begin(); it != items.end(); ++it) {
        // search for the action item.
        QString cmdName = QString::fromUtf8((*it)->command().c_str());
        QAction* action = findAction(actions, cmdName);
        if (!action) {
            int size = toolbar->actions().size(); 
            if ((*it)->command() == "Separator")
                toolbar->addSeparator();
            else 
                mgr.addTo((*it)->command().c_str(), toolbar);
            auto actions = toolbar->actions();

            // We now support single command adding multiple actions
            if (actions.size()) {
                for (int i=std::min(actions.size()-1, size); i<actions.size(); ++i)
                    actions[i]->setData(cmdName);
            }
        } else {
            do {
                // Note: For toolbars we do not remove and re-add the actions
                // because this causes flicker effects. So, it could happen that the order of
                // buttons doesn't match with the order of commands in the workbench.
                int index = actions.indexOf(action);
                actions.removeAt(index);
            } while ((*it)->command() != "Separator"
                    && (action = findAction(actions, cmdName)));
        }
    }

    // remove all tool buttons which we don't need for the moment
    for (QList<QAction*>::Iterator it = actions.begin(); it != actions.end(); ++it) {
        toolbar->removeAction(*it);
    }
}

void ToolBarManager::saveState() const
{
    // Base::ConnectionBlocker block(const_cast<ToolBarManager*>(this)->connParam);

    // ToolBar visibility is now synced using Qt signals
}

void ToolBarManager::restoreState()
{
    std::map<int, QToolBar*> sbToolBars;
    std::map<int, QToolBar*> mbRightToolBars;
    std::map<int, QToolBar*> mbLeftToolBars;
    for (auto &v : toolBars()) {
        QToolBar *toolbar = v.second;
        if (!toolbar)
            continue;
        QByteArray toolbarName = toolbar->objectName().toUtf8();
        if (toolbar->windowTitle().isEmpty() && isToolBarEmpty(toolbar)) {
            toolbar->toggleViewAction()->setVisible(false);
            setToolBarVisible(toolbar, false);
        }
        else if (this->toolbarNames.indexOf(toolbar->objectName()) >= 0)
            setToolBarVisible(toolbar, hPref->GetBool(toolbarName, toolbar->isVisible()));
        else
            setToolBarVisible(toolbar, false);
        int idx = hStatusBar->GetInt(toolbarName, -1);
        if (idx >= 0) {
            sbToolBars[idx] = toolbar;
            continue;
        }
        idx = hMenuBarLeft->GetInt(toolbarName, -1);
        if (idx >= 0) {
            mbLeftToolBars[idx] = toolbar;
            continue;
        }
        idx = hMenuBarRight->GetInt(toolbarName, -1);
        if (idx >= 0) {
            mbRightToolBars[idx] = toolbar;
            continue;
        }
        if (toolbar->parentWidget() != getMainWindow()) {
            addToolBarToMainWindow(toolbar);
        }
    }

    statusBarArea->restoreState(sbToolBars);
    menuBarRightArea->restoreState(mbRightToolBars);
    menuBarLeftArea->restoreState(mbLeftToolBars);
    restored = true;
}

void ToolBarManager::addToolBarToMainWindow(QToolBar *toolbar)
{
    Base::StateLocker guard(adding);
    getMainWindow()->addToolBar(toolbar);
}

ToolBarArea *ToolBarManager::getToolBarArea(QToolBar *toolbar)
{
    auto parent = toolbar->parentWidget();
    if (parent == statusBarArea)
        return statusBarArea;
    if (parent == menuBarLeftArea)
        return menuBarLeftArea;
    if (parent == menuBarRightArea)
        return menuBarRightArea;
    return nullptr;
}

void ToolBarManager::retranslate()
{
    for (auto &v : toolBars()) {
        QToolBar *toolbar = v.second;
        if (!toolbar) continue;
        if (toolbar->objectName().startsWith(QStringLiteral("Custom_")))
            continue;
        QByteArray toolbarName = toolbar->objectName().toUtf8();
        if (toolbar->windowTitle().size())
            toolbar->setWindowTitle(QApplication::translate("Workbench", (const char*)toolbarName));
    }
}

QAction* ToolBarManager::findAction(const QList<QAction*>& acts, const QString& item) const
{
    for (QList<QAction*>::ConstIterator it = acts.begin(); it != acts.end(); ++it) {
        if ((*it)->data().toString() == item)
            return *it;
    }

    return 0; // no item with the user data found
}

std::map<QString, QPointer<QToolBar>> ToolBarManager::toolBars()
{
    std::map<QString, QPointer<QToolBar>> res;
    auto mw = getMainWindow();
    for (auto tb : mw->findChildren<QToolBar*>()) {
        auto parent = tb->parentWidget();
        if (!parent)
            continue;
        if (parent != mw
                && parent != mw->statusBar()
                && parent != statusBarArea
                && parent != menuBarLeftArea
                && parent != menuBarRightArea)
            continue;
        QString name = tb->objectName();
        if (name.isEmpty() || name.startsWith(QStringLiteral("*")))
            continue;
        auto &p = res[name];
        if (!p) {
            p = tb;
            connectToolBar(tb);
            continue;
        }
        if (p->windowTitle().isEmpty() || tb->windowTitle().isEmpty()) {
            // We now pre-create all recorded toolbars (with an empty title)
            // so that when application starts, Qt can restore the toolbar
            // position properly. However, some user code may later create the
            // toolbar with the same name without checking existence (e.g. Draft
            // tray). So we will replace our toolbar here.
            //
            // Also, because findChildren() may return object in any order, we
            // will only replace the toolbar having an empty title with the
            // other that has a title.

            QToolBar *a = p, *b = tb;
            if (a->windowTitle().isEmpty() && isToolBarEmpty(a))
                std::swap(a, b);
            if (!a->windowTitle().isEmpty() || !isToolBarEmpty(a)) {
                // replace toolbar and insert it at the same location
                FC_TRACE("replacing " << name.toUtf8().constData() << ' ' << b << " -> " << a);
                getMainWindow()->insertToolBar(b, a);
                p = a;
                connectToolBar(a);
                if (connectedToolBars.erase(b)) {
                    b->setObjectName(QStringLiteral("__scrapped"));
                    b->deleteLater();
                }
                continue;
            }
        }

        if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
            FC_WARN("duplicate toolbar " << name.toUtf8().constData());
    }
    return res;
}

void ToolBarManager::onToggleToolBar(bool visible)
{
    auto toolbar = qobject_cast<QToolBar*>(sender());
    if (!toolbar)
        return;
    if (visible && toolbar->windowTitle().isEmpty() && isToolBarEmpty(toolbar)) {
        setToolBarVisible(toolbar, false);
        return;
    }
    if (!restored || getMainWindow()->isRestoringWindowState())
        return;

    bool enabled = visible;
    if (!visible && !toolbar->toggleViewAction()->isVisible()) {
        // This usually means the toolbar is hidden as a result of
        // workbench switch. The parameter entry however means whether
        // the toolbar is enabled while at its owner workbench
        enabled = true;
    }
    Base::ConnectionBlocker block(connParam);
    hPref->SetBool(toolbar->objectName().toUtf8(), enabled);
    auto parent = toolbar->parentWidget();
    if (parent == menuBarLeftArea || parent == menuBarRightArea) {
        menuBarTimer.start(10);
    }
}

void ToolBarManager::onToggleStatusBarWidget(QWidget *w, bool visible)
{
    Base::ConnectionBlocker block(connParam);
    w->setVisible(visible);
    hStatusBar->SetBool(w->objectName().toUtf8().constData(), w->isVisible());
}

void ToolBarManager::onMovableChanged(bool)
{
    setToolBarMovable(qobject_cast<QToolBar*>(sender()));
}

void ToolBarManager::setToolBarMovable(QToolBar *toolbar)
{
    if (!toolbar)
        return;

    bool movable = toolbar->isMovable() && isDefaultMovable();

    if (restored && !toolbar->objectName().isEmpty()) {
        Base::ConnectionBlocker block(connParam);
        hMovable->SetBool(toolbar->objectName().toUtf8(), movable);
    }
    if (auto area = getToolBarArea(toolbar)) {
        if (auto grip = toolbar->findChild<ToolBarGrip*>()) {
            if (!movable) {
                toolbar->removeAction(grip->action());
                grip->deleteLater();
            }
        }
        else {
            new ToolBarGrip(toolbar);
        }
        area->adjustParentSize();
    }
    QString name = QStringLiteral("_fc_toolbar_sep_");
    auto sep = toolbar->findChild<QAction*>(name);
    if (sep) {
        if (movable) {
            toolbar->removeAction(sep);
            sep->deleteLater();
        }
    }
    else if (!movable) {
        auto actions = toolbar->actions();
        if (actions.size()) {
            sep = toolbar->insertSeparator(actions[0]);
            sep->setObjectName(name);
        }
    }
}

void ToolBarManager::checkToolBar()
{
    if (_instance && !_instance->adding) {
        // In case some user code creates its own toolbar without going through
        // the toolbar manager, we shall call toolBars() using a timer to try
        // to hook it up. One example is 'Draft Snap'. See comment in
        // toolBars() on how we deal with this and move the toolBar to previous
        // saved position.
        _instance->timerChild.start();
    }
}

bool ToolBarManager::addToolBarToArea(QObject *o, QMouseEvent *ev)
{
    auto statusBar = getMainWindow()->statusBar();
    if (!statusBar || !statusBar->isVisible())
        statusBar = nullptr;

    auto menuBar = getMainWindow()->menuBar();
    if (!menuBar || !menuBar->isVisible()) {
        if (!statusBar)
            return false;
        menuBar = nullptr;
    }

    auto tb = qobject_cast<QToolBar*>(o);
    if (!tb || !tb->isFloating())
        return false;

    static QPointer<OverlayDragFrame> tbPlaceholder;
    static QPointer<ToolBarArea> lastArea;
    static int tbIndex = -1;
    if (ev->type() == QEvent::MouseMove) {
        if (tb->orientation() != Qt::Horizontal
            || ev->buttons() != Qt::LeftButton) {
            if (tbIndex >= 0) {
                if (lastArea) {
                    lastArea->removeWidget(tbPlaceholder);
                    lastArea = nullptr;
                }
                tbPlaceholder->hide();
                tbIndex = -1;
            }
            return false;
        }
    }

    if (ev->type() == QEvent::MouseButtonRelease && ev->button() != Qt::LeftButton)
        return false;

    QPoint pos = QCursor::pos();
    ToolBarArea *area = nullptr;
    if (statusBar) {
        QRect rect(statusBar->mapToGlobal(QPoint(0,0)), statusBar->size());
        if (rect.contains(pos))
            area = statusBarArea;
    }
    if (!area) {
        if (!menuBar) {
            return false;
        }
        QRect rect(menuBar->mapToGlobal(QPoint(0,0)), menuBar->size());
        if (rect.contains(pos)) {
            if (pos.x() - rect.left() < menuBar->width()/2) {
                area = menuBarLeftArea;
            }
            else {
                area = menuBarRightArea;
            }
        }
        else {
            if (tbPlaceholder) {
                if (lastArea) {
                    lastArea->removeWidget(tbPlaceholder);
                    lastArea = nullptr;
                }
                tbPlaceholder->hide();
                tbIndex = -1;
            }
            return false;
        }
    }

    int idx = 0;
    for (int c = area->count(); idx < c ;++idx) {
        auto w = area->widgetAt(idx);
        if (!w || w->isHidden())
            continue;
        int p = w->mapToGlobal(w->rect().center()).x();
        if (pos.x() < p)
            break;
    }
    if (tbIndex >= 0 && tbIndex == idx-1)
        idx = tbIndex;
    if (ev->type() == QEvent::MouseMove) {
        if (!tbPlaceholder) {
            tbPlaceholder = new OverlayDragFrame(getMainWindow());
            tbPlaceholder->hide();
            tbIndex = -1;
        }
        if (tbIndex != idx) {
            tbIndex = idx;
            tbPlaceholder->setSizePolicy(tb->sizePolicy());
            tbPlaceholder->setMinimumWidth(tb->minimumWidth());
            tbPlaceholder->resize(tb->size());
            area->insertWidget(idx, tbPlaceholder);
            lastArea = area;
            tbPlaceholder->adjustSize();
            tbPlaceholder->show();
        }
    } else {
        tbIndex = idx;
        QTimer::singleShot(10, tb, [tb,this]() {
            if (!lastArea)
                return;
            else {
                tbPlaceholder->hide();
                QSignalBlocker block(tb);
                lastArea->removeWidget(tbPlaceholder);
                getMainWindow()->removeToolBar(tb);
                tb->setOrientation(Qt::Horizontal);
                lastArea->insertWidget(tbIndex, tb);
                ToolBarManager::getInstance()->setToolBarVisible(tb, true);
                lastArea = nullptr;
            }
            tb->topLevelChanged(false);
            tbIndex = -1;
        });
    }
    return false;
}

bool ToolBarManager::showContextMenu(QObject *source)
{
    QMenu menu;
    QHBoxLayout *layout = nullptr;
    ToolBarArea *area;
    if (getMainWindow()->statusBar() == source) {
        area = statusBarArea;
        for (auto l : source->findChildren<QHBoxLayout*>()) {
            if(l->indexOf(area) >= 0) {
                layout = l;
                break;
            }
        }
    }
    else if (getMainWindow()->menuBar() == source) {
        QPoint pos = QCursor::pos();
        QRect rect(menuBarLeftArea->mapToGlobal(QPoint(0,0)), menuBarLeftArea->size());
        if (rect.contains(pos)) {
            area = menuBarLeftArea;
        }
        else {
            rect = QRect(menuBarRightArea->mapToGlobal(QPoint(0,0)), menuBarRightArea->size());
            if (rect.contains(pos)) {
                area = menuBarRightArea;
            }
            else {
                return false;
            }
        }
    }
    else {
        return false;
    }
    auto tooltip = QObject::tr("Toggles visibility");
    QCheckBox *checkbox;

    auto addMenuVisibleItem = [&](QToolBar *toolbar, int, ToolBarArea *) {
        auto action = toolbar->toggleViewAction();
        if ((action->isVisible() || toolbar->isVisible()) && action->text().size()) {
            action->setVisible(true);
            auto wa = Action::addCheckBox(&menu, action->text(),
                    tooltip, action->icon(), action->isChecked(), &checkbox);
            QObject::connect(checkbox, SIGNAL(toggled(bool)), action, SIGNAL(triggered(bool)));
            QObject::connect(wa, SIGNAL(triggered(bool)), action, SIGNAL(triggered(bool)));
        }
    };

    if (layout) {
        for (int i = 0, c = layout->count(); i < c; ++i) {
            auto w = layout->itemAt(i)->widget();
            if (!w || w == area
                || w->objectName().isEmpty()
                || w->objectName().startsWith(QStringLiteral("*")))
            {
                continue;
            }
            QString name = w->windowTitle();
            if (name.isEmpty()) {
                name = w->objectName();
                name.replace(QLatin1Char('_'), QLatin1Char(' '));
                name = name.simplified();
            }
            auto wa = Action::addCheckBox(&menu, name,
                    tooltip, QIcon(), w->isVisible(), &checkbox);
            auto onToggle = [w, this](bool visible) {onToggleStatusBarWidget(w, visible);};
            QObject::connect(checkbox, &QCheckBox::toggled, onToggle);
            QObject::connect(wa, &QAction::triggered, onToggle);
        }
    }

    area->foreachToolBar(addMenuVisibleItem);
    menu.exec(QCursor::pos());
    return true;
}

bool ToolBarManager::eventFilter(QObject *o, QEvent *e)
{
    bool res = false;
    switch(e->type()) {
    case QEvent::MouseButtonRelease: {
        auto mev = static_cast<QMouseEvent*>(e);
        if (mev->button() == Qt::RightButton) {
            if (showContextMenu(o)) {
                return true;
            }
        }
    }
    // fall through
    case QEvent::MouseMove:
        res = addToolBarToArea(o, static_cast<QMouseEvent*>(e));
        break;
    case QEvent::ChildAdded:
        if (auto tb = qobject_cast<QToolBar*>(o)) {
            if (tb->toggleViewAction()->isVisible()
                || !globalToolBarNames.count(tb->objectName()))
                break;
            QByteArray name = tb->objectName().toUtf8();
            if (!hGlobal->HasGroup(name.constData()))
                break;
            std::string toolbarName = hGlobal->GetGroup(name.constData())->GetASCII("Name");

            tb->toggleViewAction()->setVisible(true);
            if (tb->windowTitle().isEmpty())
                tb->setWindowTitle(QApplication::translate("Workbench", toolbarName.c_str()));
            if (!tb->isVisible() && hPref->GetBool(toolbarName.c_str(), true))
                tb->setVisible(true);
        }
        break;
    default:
        break;
    }
    return res;
}

void ToolBarManager::setToolBarVisibility(bool show, const QList<QString>& names)
{
    auto toolbars = toolBars();
    for (auto& name : names) {
        auto it = toolbars.find(name);
        if (it == toolbars.end() || !it->second)
            continue;
        QToolBar *tb = it->second;
        if (show) {
            if(hPref->GetBool(name.toStdString().c_str(), true))
                tb->show();
            tb->toggleViewAction()->setVisible(true);
        }
        else {
            tb->hide();
            tb->toggleViewAction()->setVisible(false);
        }
    }
}

QSize ToolBarManager::actionsIconSize(const QList<QAction*> &actions, QWidget *widget)
{
    int s = getInstance()->toolBarIconSize(widget);
    QSize iconSize(s, s);
    for (auto action : actions) {
        if (!action->isVisible())
            continue;
        auto icon = action->icon();
        if (icon.isNull())
            continue;
        auto size = icon.actualSize(iconSize);
        if (size.height() < iconSize.height()) {
            iconSize.setWidth(static_cast<float>(iconSize.height())/size.height()*size.width());
        }
    }
    return iconSize;
}

void ToolBarManager::checkToolBarIconSize(QToolBar *tb)
{
    if (!tb)
        return;
    auto &v = resizingToolBars[tb];
    if (!v) {
        timerResize.start(100);
        v = tb;
    }
}

void ToolBarManager::checkToolBarIconSize(QAction *action)
{
    for (auto w : action->associatedWidgets()) {
        if (auto tb = qobject_cast<QToolBar*>(w))
            checkToolBarIconSize(tb);
    }
}

void ToolBarManager::setupToolBarIconSize()
{
    int s = toolBarIconSize();
    getMainWindow()->setIconSize(QSize(s, s));
    // Most of the the toolbar will have explicit icon size, so the above call
    // to QMainWindow::setIconSize() will have no effect. We need to explicitly
    // change the icon size.
    for (auto toolbar : getMainWindow()->findChildren<QToolBar*>()) {
        setToolBarIconSize(toolbar);
    }
}

#include "moc_ToolBarManager.cpp"
