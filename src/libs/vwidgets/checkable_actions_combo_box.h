//  @file   checkable_actions_combo_box.h
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

#ifndef CHECKABLE_ACTIONS_COMBO_BOX_H
#define CHECKABLE_ACTIONS_COMBO_BOX_H

#include <QComboBox>

class QAction;
class QMenu;

/**
 * @brief The VCheckableActionsComboBox class a combo box whose list is a menu of check boxes of checkable actions.
 *
 * The box looks exactly like the other combo boxes and always shows the same caption. A click on it opens
 * a menu with the actions (the same look as the list of any combo box on the platform). A click on an item
 * switches the action and the menu stays open, so several items can be ticked in one go.
 */
class VCheckableActionsComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit VCheckableActionsComboBox(const QString &caption, QWidget *parent = nullptr);

    void           addCheckableAction(QAction *action);

    virtual void   showPopup() override;
    virtual void   hidePopup() override;

private:
    Q_DISABLE_COPY(VCheckableActionsComboBox)

    QMenu *m_menu;
};

#endif // CHECKABLE_ACTIONS_COMBO_BOX_H
