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

#ifndef MOVELENGTHLABEL_H
#define MOVELENGTHLABEL_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "moveabstractlabel.h"

/**
 * @brief The MoveLengthLabel class undo command: the user dragged the length label of a straight line.
 *
 * The position is the offset of the label from the middle of the line (in pixels).
 */
class MoveLengthLabel : public MoveAbstractLabel
{
public:
                    MoveLengthLabel(VAbstractPattern *doc, const QPointF &pos, const quint32 &id,
                                    QUndoCommand *parent = nullptr);
    virtual        ~MoveLengthLabel()=default;

    virtual bool    mergeWith(const QUndoCommand *command) override;
    virtual int     id() const override;

protected:
    virtual void    Do(const QPointF &pos) override;

private:
    Q_DISABLE_COPY(MoveLengthLabel)
};

#endif // MOVELENGTHLABEL_H
