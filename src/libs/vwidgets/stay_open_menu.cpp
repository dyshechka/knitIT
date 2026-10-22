//  @file   stay_open_menu.cpp
//  @brief  Keeps a menu with checkable items open while the items are clicked.
//---------------------------------------------------------------------------------------------------------------------
/************************************************************************
 **
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "stay_open_menu.h"

#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>

//---------------------------------------------------------------------------------------------------------------------
VStayOpenMenuFilter::VStayOpenMenuFilter(QMenu *menu)
    : QObject(menu)
{
    menu->installEventFilter(this);
}

//---------------------------------------------------------------------------------------------------------------------
void VStayOpenMenuFilter::install(QMenu *menu)
{
    if (menu != nullptr)
    {
        new VStayOpenMenuFilter(menu);   // belongs to the menu and is deleted with it
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VStayOpenMenuFilter::eventFilter(QObject *watched, QEvent *event)
{
    QMenu *menu = qobject_cast<QMenu *>(watched);
    if (menu == nullptr)
    {
        return QObject::eventFilter(watched, event);
    }

    QAction *action = nullptr;
    if (event->type() == QEvent::MouseButtonRelease)
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            action = menu->actionAt(mouseEvent->pos());
        }
    }
    else if (event->type() == QEvent::KeyPress)
    {
        const int key = static_cast<QKeyEvent *>(event)->key();
        if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space)
        {
            action = menu->activeAction();
        }
    }

    if (action != nullptr && action->isCheckable() && action->isEnabled() && action->menu() == nullptr)
    {
        action->trigger();   // switch the item, but do not let the menu close
        return true;
    }

    return QObject::eventFilter(watched, event);
}
