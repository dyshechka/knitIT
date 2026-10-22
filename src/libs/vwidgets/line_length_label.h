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

#ifndef LINE_LENGTH_LABEL_H
#define LINE_LENGTH_LABEL_H

#include <QGraphicsItem>
#include <QColor>
#include <QFont>
#include <QLineF>
#include <QPainterPath>
#include <QRectF>
#include <functional>
#include <QString>

/**
 * @brief The VLineLengthLabel class text label placed at the middle of a straight line.
 *
 * The label is scaled the same way as the point names are. It does not take mouse events itself,
 * the view treats a click on it as a click on its line.
 * The label can be dragged, the new offset from the middle of the line is reported to the owner of the label.
 * A label that is far from the middle of its line gets a thin leader line to the middle, like a point name.
 */
class VLineLengthLabel : public QGraphicsItem
{
public:
    enum { Type = UserType + 1000 };

    explicit VLineLengthLabel(QGraphicsItem *parent = nullptr);

    virtual int    type() const override {return Type;}

    void           setLabel(const QString &text, const QPointF &center, const QPointF &offset = QPointF());
    void           setMovedHandler(const std::function<void(const QPointF &)> &handler);
    void           setMovable(bool movable);

    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;
    virtual void   paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

protected:
    virtual void   mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void   mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void   mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Q_DISABLE_COPY(VLineLengthLabel)

    QString        m_text;
    QRectF         m_rect;
    QPointF        m_center;      // natural place of the label: the middle of the line
    QPointF        m_offset;      // how far the user has moved the label from the middle of the line
    QPointF        m_pressPos;    // position of the label when the mouse button was pressed
    QPointF        m_pressScenePos; // place of the mouse in the scene when the button was pressed
    bool           m_movable;
    std::function<void(const QPointF &)> m_moved;

    // Everything that is needed to draw the label is worked out once, not at every repaint.
    QFont          m_font;
    QColor         m_textColor;
    QColor         m_leaderColor;
    QPainterPath   m_shape;
    bool           m_hasLeader;
    QLineF         m_leader;

    void           refreshLeader();
};

#endif // LINE_LENGTH_LABEL_H
