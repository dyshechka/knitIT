/************************************************************************
 **
 **  @file   checkbox_property.h
 **  @brief  Property edited with a check box.
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

#ifndef CHECKBOX_PROPERTY_H
#define CHECKBOX_PROPERTY_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "../vproperty.h"

namespace VPE
{

//! The CheckBoxProperty is a yes/no property that is edited with a check box. Its value is a bool.
class CheckBoxProperty : public VProperty
{
    Q_OBJECT
public:
    explicit CheckBoxProperty(const QString &name);
    virtual ~CheckBoxProperty() override {}

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                  const QAbstractItemDelegate *delegate) override;
    virtual QVariant getEditorData(const QWidget *editor) const override;
    virtual void     setValue(const QVariant &value) override;
    virtual QString  type() const override;
    Q_REQUIRED_RESULT virtual VProperty *clone(bool include_children = true,
                                               VProperty *container = nullptr) const override;

public slots:
    void             toggled(bool checked);

private:
    Q_DISABLE_COPY(CheckBoxProperty)
};

}

#endif // CHECKBOX_PROPERTY_H
