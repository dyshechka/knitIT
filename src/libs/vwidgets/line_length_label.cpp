//  @file   line_length_label.h
//  @brief  Label with the length of a straight line (name, formula and length) drawn on the pattern.
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

#include "line_length_label.h"

#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>
#include <QPen>

#include "global.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

namespace
{
    const qreal labelPadding = 2.0;
    const qreal minLeaderLength = 12.0;   // a leader line shorter than this is only noise
}

//---------------------------------------------------------------------------------------------------------------------
VLineLengthLabel::VLineLengthLabel(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_text()
    , m_rect()
    , m_pressPos()
    , m_pressScenePos()
    , m_movable(true)
    , m_font()
    , m_textColor()
    , m_leaderColor()
    , m_shape()
    , m_hasLeader(false)
    , m_leader()
{
    // The label is dragged by its own code, not by ItemIsMovable: Qt moves all selected items together,
    // so a selected point would be dragged along with the label.
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(100);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setMovedHandler set the function that is called when the user has dragged the label.
void VLineLengthLabel::setMovedHandler(const std::function<void(const QPointF &)> &handler)
{
    m_moved = handler;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setMovable allow or forbid to drag the label. A label that can not be dragged lets the clicks go through.
void VLineLengthLabel::setMovable(bool movable)
{
    m_movable = movable;
    setAcceptedMouseButtons(movable ? Qt::LeftButton : Qt::NoButton);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setLabel set the text and the position of the label.
/// @param text text, lines are separated by a new line character.
/// @param center middle of the line in the coordinates of the parent item.
/// @param offset how far the label is moved from the middle of the line.
void VLineLengthLabel::setLabel(const QString &text, const QPointF &center, const QPointF &offset)
{
    prepareGeometryChange();
    m_text = text;

    // Same font and colors as the point names use. Read here, not at every repaint: settings are slow.
    m_font = qApp->Settings()->getPointNameFont();
    m_font.setPointSize(qApp->Settings()->getPointNameSize());
    m_textColor = QColor(qApp->Settings()->getPointNameColor());
    m_textColor.setAlpha(224);
    m_leaderColor = QColor(qApp->Settings()->getPointNameColor());
    m_leaderColor.setAlpha(128);

    const QFontMetricsF metrics(m_font);
    const QRectF textRect = metrics.boundingRect(QRectF(0, 0, 10000, 10000), Qt::AlignLeft, m_text);
    m_rect = QRectF(-textRect.width() / 2.0 - labelPadding, -textRect.height() / 2.0 - labelPadding,
                    textRect.width() + 2 * labelPadding, textRect.height() + 2 * labelPadding);

    m_shape = QPainterPath();
    m_shape.addRect(m_rect);

    m_center = center;
    m_offset = offset;
    setPos(m_center + m_offset);
    refreshLeader();
    update();
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VLineLengthLabel::boundingRect() const
{
    // A little reserve around: antialiased text and the leader line are drawn slightly outside of the exact rectangle.
    if (m_hasLeader)
    {
        const QRectF leaderRect = QRectF(m_leader.p1(), m_leader.p2()).normalized();
        return m_rect.united(leaderRect).adjusted(-2, -2, 2, 2);
    }
    return m_rect.adjusted(-2, -2, 2, 2);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief shape only the text takes mouse clicks, not the leader line.
QPainterPath VLineLengthLabel::shape() const
{
    return m_shape;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief refreshLeader work out the leader line from the label to the middle of its line (in the coordinates
/// of the label). There is none if the label is close to the middle of the line.
/// Must be called every time the position, the scale or the size of the label changes.
void VLineLengthLabel::refreshLeader()
{
    m_hasLeader = false;
    if (parentItem() == nullptr)
    {
        return;
    }

    const QPointF target = mapFromParent(m_center);
    if (m_rect.contains(target))
    {
        return;
    }

    // The point where the ray from the label center to the target leaves the label.
    const QLineF ray(m_rect.center(), target);
    const QLineF edges[] = {QLineF(m_rect.topLeft(), m_rect.topRight()), QLineF(m_rect.topRight(), m_rect.bottomRight()),
                            QLineF(m_rect.bottomRight(), m_rect.bottomLeft()),
                            QLineF(m_rect.bottomLeft(), m_rect.topLeft())};
    QPointF start = m_rect.center();
    for (const QLineF &edge : edges)
    {
        QPointF crossing;
        if (ray.intersects(edge, &crossing) == QLineF::BoundedIntersection)
        {
            start = crossing;
            break;
        }
    }

    m_leader = QLineF(start, target);
    m_hasLeader = m_leader.length() > minLeaderLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VLineLengthLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Zoomed far out the text is smaller than a pixel: same shortcut as the point names have.
    if (option->levelOfDetailFromTransform(painter->worldTransform()) < 0.1)
    {
        return;
    }

    // The same behaviour as the point names have: keep the size when the pattern is zoomed in,
    // and shrink together with the pattern when it is zoomed out.
    const qreal viewScale = sceneScale(scene());
    const qreal wanted = viewScale > 1.0 ? 1.0 / viewScale : 1.0;
    if (not qFuzzyCompare(scale(), wanted))
    {
        setScale(wanted);
        refreshLeader();
    }

    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    if (m_hasLeader)
    {
        QPen leaderPen(m_leaderColor, 1);
        leaderPen.setCosmetic(true);
        painter->setPen(leaderPen);
        painter->drawLine(m_leader);
    }

    painter->setFont(m_font);
    painter->setPen(m_textColor);
    painter->drawText(m_rect, Qt::AlignCenter, m_text);   // every line is centered, lines are split by '\n'
    painter->restore();
}

//---------------------------------------------------------------------------------------------------------------------
void VLineLengthLabel::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (not m_movable || event->button() != Qt::LeftButton)
    {
        event->ignore();
        return;
    }
    m_pressPos = pos();
    m_pressScenePos = event->scenePos();
    event->accept();
}

//---------------------------------------------------------------------------------------------------------------------
void VLineLengthLabel::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (not m_movable)
    {
        return;
    }
    prepareGeometryChange();   // the leader line follows the label

    QPointF delta = event->scenePos() - m_pressScenePos;
    if (parentItem() != nullptr)
    {
        delta = parentItem()->mapFromScene(event->scenePos()) - parentItem()->mapFromScene(m_pressScenePos);
    }
    setPos(m_pressPos + delta);
    refreshLeader();
    update();                  // the leader line has just changed: draw the new one, wipe the old one
}

//---------------------------------------------------------------------------------------------------------------------
void VLineLengthLabel::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();

    if (m_movable && (pos() - m_pressPos).manhattanLength() > 0.5)
    {
        m_offset = pos() - m_center;
        if (m_moved)
        {
            m_moved(m_offset);
        }
    }
}
