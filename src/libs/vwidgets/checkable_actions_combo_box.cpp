//  @file   checkable_actions_combo_box.cpp
//  @brief  A combo box that opens a menu of check boxes (bound to actions) that stays open.
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

#include "checkable_actions_combo_box.h"

#include <QAction>
#include <QMenu>

#include "stay_open_menu.h"

//---------------------------------------------------------------------------------------------------------------------
VCheckableActionsComboBox::VCheckableActionsComboBox(const QString &caption, QWidget *parent)
    : QComboBox(parent)
    , m_menu(new QMenu(this))
{
    addItem(caption);   // the only item, it is always shown in the box
    setCurrentIndex(0);
    VStayOpenMenuFilter::install(m_menu);
}

//---------------------------------------------------------------------------------------------------------------------
void VCheckableActionsComboBox::addCheckableAction(QAction *action)
{
    if (action != nullptr)
    {
        m_menu->addAction(action);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VCheckableActionsComboBox::showPopup()
{
    if (m_menu->isVisible())
    {
        m_menu->hide();
        return;
    }
    m_menu->popup(mapToGlobal(QPoint(0, height())));
}

//---------------------------------------------------------------------------------------------------------------------
void VCheckableActionsComboBox::hidePopup()
{
    m_menu->hide();
}
