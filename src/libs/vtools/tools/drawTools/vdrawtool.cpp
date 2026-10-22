/***************************************************************************
 **  @file   vdrawtool.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **  @file   vdrawtool.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vdrawtool.h"

#include "../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vmisc/logging.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/variables/vlinelength.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vmisc/vcommonsettings.h"
#include "../vwidgets/line_length_label.h"
#include "../../undocommands/addtocalc.h"
#include "../../undocommands/label/movelengthlabel.h"
#include "../../undocommands/savetooloptions.h"

#include <QDialog>
#include <QDomNode>
#include <QMessageLogger>
#include <QScopedPointer>
#include <QStringList>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QUndoStack>
#include <Qt>
#include <QtDebug>

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VDrawTool constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 */
VDrawTool::VDrawTool(VAbstractPattern *doc, VContainer *data, quint32 id, QObject *parent)
    : VInteractiveTool(doc, data, id, parent)
    , activeBlockName(doc->getActiveDraftBlockName())
    , m_lineType(LineTypeSolidLine)
    , m_lineWeight(DefaultLineWeight)
    , m_showLength(false)
    , m_lengthLabel(nullptr)
    , m_lengthLabelOffset()
    , m_lengthLabelOffsetLoaded(false)
    , m_lengthLabelMovable(true)
    , m_formulaForUser()
    , m_formulaForUserSeparator(false)
{
    connect(this->doc, &VAbstractPattern::activeDraftBlockChanged, this, &VDrawTool::activeBlockChanged);
    connect(this->doc, &VAbstractPattern::draftBlockNameChanged, this, &VDrawTool::blockNameChanged);
    connect(this->doc, &VAbstractPattern::ShowTool, this, &VDrawTool::ShowTool);
    connect(this->doc, &VAbstractPattern::lengthLabelsChanged, this, &VDrawTool::updateLengthLabel);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief isUsed check if the tool can't be deleted.
///
/// Besides the id based reference count, a tool can also be needed through the variables it registered in the
/// data container, for example the length of an arc or the angles of a spline. Formulas reference those variables
/// by name, so the reference counter never sees them. VInternalVariable::Filter() tells which variables belong to
/// this tool, the same test the formula dialog uses to hide a tool's own variables.
///
/// @return true if another tool references this tool by id, or a formula uses one of its variables.
//---------------------------------------------------------------------------------------------------------------------

bool VDrawTool::isUsed() const
{
    if (VInteractiveTool::isUsed())
    {
        return true;
    }

    QStringList variable_names;
    const QHash<QString, QSharedPointer<VInternalVariable>> *variables = data.DataVariables();
    for (auto i = variables->constBegin(); i != variables->constEnd(); ++i)
    {
        if (i.value()->Filter(m_id))
        {
            variable_names.append(i.key());
        }
    }

    return doc->isVariableUsed(variable_names);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowTool  highlight tool.
 * @param id object id in container.
 * @param enable enable or disable highlight.
 */
void VDrawTool::ShowTool(quint32 id, bool enable)
{
    Q_UNUSED(id)
    Q_UNUSED(enable)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief activeBlockChanged disable or enable context menu after change active draft block.
 * @param newName new name active draft block.
 */
void VDrawTool::activeBlockChanged(const QString &newName)
{
    Disable(!(activeBlockName == newName), newName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief blockNameChanged save new name active draft block.
 * @param oldName old name.
 * @param newName new name active draft block. new name.
 */
void VDrawTool::blockNameChanged(const QString &oldName, const QString &newName)
{
    if (activeBlockName == oldName)
    {
        activeBlockName = newName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveDialogChange()
{
    qCDebug(vTool, "Saving tool options after using dialog");
    QDomElement oldDomElement = doc->elementById(m_id, getTagName());
    if (oldDomElement.isElement())
    {
        QDomElement newDomElement = oldDomElement.cloneNode().toElement();
        SaveDialog(newDomElement);

        SaveToolOptions *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        connect(saveOptions, &SaveToolOptions::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
        qApp->getUndoStack()->push(saveOptions);
    }
    else
    {
        qCWarning(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with Information about tool into file.
 */
void VDrawTool::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOption(QSharedPointer<VGObject> &obj)
{
    qCDebug(vTool, "Saving tool options");
    QDomElement oldDomElement = doc->elementById(m_id, getTagName());
    if (oldDomElement.isElement())
    {
        QDomElement newDomElement = oldDomElement.cloneNode().toElement();

        SaveOptions(newDomElement, obj);

        SaveToolOptions *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        connect(saveOptions, &SaveToolOptions::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
        qApp->getUndoStack()->push(saveOptions);
    }
    else
    {
        qCWarning(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    Q_UNUSED(obj)

    doc->SetAttribute(tag, VDomDocument::AttrId, m_id);

    SaveShowLength(tag, m_showLength);
    SaveLengthLabelOffset(tag, m_lengthLabelOffset);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveShowLength write the "show length" flag of a line into its tag.
 *
 * The attribute is written only when switched on, so files without length labels stay exactly as before.
 */
void VDrawTool::SaveShowLength(QDomElement &tag, bool show)
{
    if (show)
    {
        tag.setAttribute(AttrShowLength, trueStr);
    }
    else
    {
        tag.removeAttribute(AttrShowLength);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveLengthLabelOffset write how far the length label is moved from the line middle into the tag.
 *
 * Like for the point names the values are in the pattern units. Nothing is written for a label that was not moved.
 */
void VDrawTool::SaveLengthLabelOffset(QDomElement &tag, const QPointF &offset)
{
    if (offset.isNull())
    {
        tag.removeAttribute(AttrLengthMx);
        tag.removeAttribute(AttrLengthMy);
    }
    else
    {
        tag.setAttribute(AttrLengthMx, QString().setNum(qApp->fromPixel(offset.x())));
        tag.setAttribute(AttrLengthMy, QString().setNum(qApp->fromPixel(offset.y())));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VDrawTool::makeToolTip() const
{
    return QString();
}

//---------------------------------------------------------------------------------------------------------------------
bool VDrawTool::CorrectDisable(bool disable, const QString &draftBlockName) const
{
    if (disable)
    {
        return disable;
    }
    else
    {
        return !(activeBlockName == draftBlockName);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ReadAttributes()
{
    const QDomElement domElement = doc->elementById(m_id, getTagName());
    if (domElement.isElement())
    {
        ReadToolAttributes(domElement);
        m_showLength = VDomDocument::getParameterBool(domElement, AttrShowLength, falseStr);
        m_lengthLabelOffset = QPointF(qApp->toPixel(VDomDocument::GetParametrDouble(domElement, AttrLengthMx, "0.0")),
                                      qApp->toPixel(VDomDocument::GetParametrDouble(domElement, AttrLengthMy, "0.0")));
        m_lengthLabelOffsetLoaded = true;
    }
    else
    {
        qCWarning(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::updatePointNameVisibility(quint32 id, bool visible)
{
    Q_UNUSED(id)
    Q_UNUSED(visible)
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::EnableToolMove(bool move)
{
    enableLengthLabelMove(move);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief enableLengthLabelMove the label can be dragged only while the arrow tool is active.
void VDrawTool::enableLengthLabelMove(bool move)
{
    m_lengthLabelMovable = move;
    if (m_lengthLabel != nullptr)
    {
        m_lengthLabel->setMovable(move);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief lengthLabelMoved the user has dragged the label, save the new place (can be undone).
void VDrawTool::lengthLabelMoved(const QPointF &offset)
{
    qApp->getUndoStack()->push(new MoveLengthLabel(doc, offset, m_id));
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VDrawTool::lengthLabelOffset() const
{
    return m_lengthLabelOffset;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::setLengthLabelOffset(const QPointF &offset)
{
    m_lengthLabelOffset = offset;
    m_lengthLabelOffsetLoaded = true;
    updateLengthLabel();
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::piecesMode(bool mode)
{
    Q_UNUSED(mode)
    // Do nothing.
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief updateLengthLabel redraw the length label. Tools with a straight line override it.
void VDrawTool::updateLengthLabel()
{
    // Do nothing.
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief refreshLengthLabel show, update or remove the label with the length of a straight line.
///
/// The label is shown only if the tool has the "Show length" option and the length labels are switched on
/// in the View menu. What the label contains (name, formula, length) is the same for all lines and is also
/// set in the View menu.
/// @param parent item the label belongs to (the line item).
/// @param firstPointId id of the first point of the line.
/// @param secondPointId id of the second point of the line.
/// @param internalFormula formula of the line length in the internal notation, empty if the line has none.
/// @param center middle of the line in the coordinates of the parent item.
void VDrawTool::refreshLengthLabel(QGraphicsItem *parent, quint32 firstPointId, quint32 secondPointId,
                                   const QString &internalFormula, const QPointF &center)
{
    if (not m_showLength && m_lengthLabel == nullptr)
    {// Most lines have no label: nothing to do, and no time lost on it.
        return;
    }

    const VCommonSettings *settings = qApp->Settings();

    if (not m_lengthLabelOffsetLoaded)
    {// The tool is created from a file: the place of the label is already there.
        const QDomElement domElement = doc->elementById(m_id, getTagName());
        if (domElement.isElement())
        {
            m_lengthLabelOffset = QPointF(qApp->toPixel(VDomDocument::GetParametrDouble(domElement, AttrLengthMx,
                                                                                         "0.0")),
                                          qApp->toPixel(VDomDocument::GetParametrDouble(domElement, AttrLengthMy,
                                                                                         "0.0")));
            m_lengthLabelOffsetLoaded = true;
        }
    }

    QStringList lines;
    if (m_showLength && settings->getShowLineLengths())
    {
        const QSharedPointer<VPointF> first = VAbstractTool::data.GeometricObject<VPointF>(firstPointId);
        const QSharedPointer<VPointF> second = VAbstractTool::data.GeometricObject<VPointF>(secondPointId);

        if (settings->getLineLengthShowName())
        {
            const VLengthLine line(first.data(), firstPointId, second.data(), secondPointId,
                                   *VAbstractTool::data.GetPatternUnit());
            lines.append(qApp->translateVariables()->VarToUser(line.GetName()));
        }

        // A plain number is not worth showing as a formula, the length itself says the same.
        static const QRegularExpression plainNumber(QStringLiteral("^\\s*[+-]?(\\d+\\.?\\d*|\\.\\d+)\\s*$"));
        if (settings->getLineLengthShowFormula() && not internalFormula.isEmpty()
            && not plainNumber.match(internalFormula).hasMatch())
        {
            // Translating a formula means parsing it, so the result is kept until the formula changes.
            const bool osSeparator = settings->getOsSeparator();
            if (m_formulaForUser.first != internalFormula || m_formulaForUserSeparator != osSeparator)
            {
                m_formulaForUser = qMakePair(internalFormula,
                                             qApp->translateVariables()->FormulaToUser(internalFormula, osSeparator));
                m_formulaForUserSeparator = osSeparator;
            }
            lines.append(m_formulaForUser.second);
        }

        if (settings->getLineLengthShowValue())
        {
            const QLineF line(static_cast<QPointF>(*first), static_cast<QPointF>(*second));
            const qreal length = qApp->fromPixel(line.length());
            lines.append(qApp->LocaleToString(qRound(length * 100.0) / 100.0));
        }
    }

    if (lines.isEmpty())
    {
        delete m_lengthLabel;
        m_lengthLabel = nullptr;
        return;
    }

    if (m_lengthLabel == nullptr)
    {
        m_lengthLabel = new VLineLengthLabel(parent);
        m_lengthLabel->setMovable(m_lengthLabelMovable);
        m_lengthLabel->setMovedHandler([this](const QPointF &offset) { lengthLabelMoved(offset); });
    }
    m_lengthLabel->setLabel(lines.join(QChar('\n')), center, m_lengthLabelOffset);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToCalculation add tool to calculation tag in pattern file.
 * @param domElement tag in xml tree.
 */
void VDrawTool::AddToCalculation(const QDomElement &domElement)
{
    AddToCalc *addToCal = new AddToCalc(domElement, doc);
    connect(addToCal, &AddToCalc::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(addToCal);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::addDependence(QList<quint32> &list, quint32 objectId) const
{
    if (objectId != NULL_ID)
    {
        auto originPoint = VAbstractTool::data.GetGObject(objectId);
        list.append(originPoint->getIdTool());
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VDrawTool::getLineType() const
{
    return m_lineType;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::setLineType(const QString &value)
{
    m_lineType = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VDrawTool::getLineWeight() const
{
    return m_lineWeight;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::setLineWeight(const QString &value)
{
    m_lineWeight = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
bool VDrawTool::getShowLength() const
{
    return m_showLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::setShowLength(bool value)
{
    m_showLength = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
bool VDrawTool::isPointNameVisible(quint32 id) const
{
    Q_UNUSED(id)
    return false;
}
