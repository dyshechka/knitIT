//  @file   stay_open_menu.h
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

#ifndef STAY_OPEN_MENU_H
#define STAY_OPEN_MENU_H

#include <QObject>

class QMenu;

/**
 * @brief The VStayOpenMenuFilter class makes a menu stay open when a checkable item is clicked.
 *
 * Usually a menu closes after every click, so to tick three items it has to be opened three times.
 * Install the filter on a menu and a click (or Enter / Space) on a checkable item only switches the item,
 * the menu stays open until the user clicks somewhere else, on a normal item or presses Escape.
 */
class VStayOpenMenuFilter : public QObject
{
    Q_OBJECT
public:
    static void install(QMenu *menu);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit VStayOpenMenuFilter(QMenu *menu);
    Q_DISABLE_COPY(VStayOpenMenuFilter)
};

#endif // STAY_OPEN_MENU_H
