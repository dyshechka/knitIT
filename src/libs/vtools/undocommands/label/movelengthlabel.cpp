/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   movelabel.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 10, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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

#include "movelengthlabel.h"

#include <QDomElement>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/drawTools/vdrawtool.h"
#include "../vundocommand.h"
#include "moveabstractlabel.h"

//---------------------------------------------------------------------------------------------------------------------
MoveLengthLabel::MoveLengthLabel(VAbstractPattern *doc, const QPointF &pos, const quint32 &id, QUndoCommand *parent)
    : MoveAbstractLabel(doc, id, pos, parent)
{
    setText(tr("move line length label"));

    const QDomElement domElement = doc->elementById(nodeId);
    if (domElement.isElement())
    {
        m_oldPos.rx() = qApp->toPixel(doc->GetParametrDouble(domElement, AttrLengthMx, "0.0"));
        m_oldPos.ry() = qApp->toPixel(doc->GetParametrDouble(domElement, AttrLengthMy, "0.0"));
    }
    else
    {
        qCDebug(vUndo, "Can't find line with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool MoveLengthLabel::mergeWith(const QUndoCommand *command)
{
    const MoveLengthLabel *moveCommand = static_cast<const MoveLengthLabel *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->GetPointId() != nodeId)
    {
        return false;
    }

    m_newPos = moveCommand->GetNewPos();

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int MoveLengthLabel::id() const
{
    return static_cast<int>(UndoCommand::MoveLengthLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveLengthLabel::Do(const QPointF &pos)
{
    QDomElement domElement = doc->elementById(nodeId);
    if (domElement.isElement())
    {
        VDrawTool::SaveLengthLabelOffset(domElement, pos);

        if (VDrawTool *tool = qobject_cast<VDrawTool *>(VAbstractPattern::getTool(nodeId)))
        {
            tool->setLengthLabelOffset(pos);
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find line with id = %u.", nodeId);
    }
}
