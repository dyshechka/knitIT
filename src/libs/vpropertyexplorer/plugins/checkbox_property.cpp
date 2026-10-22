/************************************************************************
 **
 **  @file   checkbox_property.cpp
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

#include "checkbox_property.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QSignalBlocker>

#include "../vproperty_p.h"

VPE::CheckBoxProperty::CheckBoxProperty(const QString &name)
    : VProperty(name, QVariant::Bool)
{
    d_ptr->VariantValue.setValue(false);
    d_ptr->VariantValue.convert(QVariant::Bool);
}

//! Returns a check box. A click on it is reported to the form the same way as in the other properties.
QWidget *VPE::CheckBoxProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                             const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    QCheckBox *editor = new QCheckBox(getName(), parent);   // the name is the text next to the box
    editor->setChecked(d_ptr->VariantValue.toBool());
    connect(editor, &QCheckBox::toggled, this, &CheckBoxProperty::toggled);

    d_ptr->editor = editor;
    return d_ptr->editor;
}

QVariant VPE::CheckBoxProperty::getEditorData(const QWidget *editor) const
{
    const QCheckBox *checkBox = qobject_cast<const QCheckBox *>(editor);
    if (checkBox != nullptr)
    {
        return checkBox->isChecked();
    }

    return QVariant(false);
}

void VPE::CheckBoxProperty::setValue(const QVariant &value)
{
    d_ptr->VariantValue = value.toBool();

    QCheckBox *checkBox = qobject_cast<QCheckBox *>(d_ptr->editor);
    if (checkBox != nullptr)
    {
        const QSignalBlocker blocker(checkBox);  // Only a click of the user must be reported.
        checkBox->setChecked(d_ptr->VariantValue.toBool());
    }
}

QString VPE::CheckBoxProperty::type() const
{
    return QStringLiteral("checkbox");
}

VPE::VProperty *VPE::CheckBoxProperty::clone(bool include_children, VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new CheckBoxProperty(getName()));
}

void VPE::CheckBoxProperty::toggled(bool checked)
{
    Q_UNUSED(checked)
    UserChangeEvent *event = new UserChangeEvent();
    QCoreApplication::postEvent(d_ptr->editor, event);
}
