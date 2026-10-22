/******************************************************************************
*   @file   pattern_old_format_converter.cpp
**  @brief  Converts an old-format pattern (.sm2d) to the current format, together with
**          the measurements file it uses (see measurements_old_format_converter.h).
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2024 Seamly2D project
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
#include "pattern_old_format_converter.h"

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QDomAttr>
#include <QDomComment>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QDomNodeList>
#include <QDomText>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QPair>

#include "measurements_old_format_converter.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vmisc/projectversion.h"

//---------------------------------------------------------------------------------------------------------------------
QString OldFormatPatternConverter::MeasurementsReference(const QString &patternPath)
{
    QFile file(patternPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }

    QDomDocument doc;
    const bool parsed = static_cast<bool>(doc.setContent(&file));
    file.close();
    if (!parsed)
    {
        return QString();
    }

    const QDomNodeList list = doc.documentElement().elementsByTagName(QStringLiteral("measurements"));
    if (list.isEmpty())
    {
        return QString();
    }
    return list.at(0).toElement().text().trimmed();
}

//---------------------------------------------------------------------------------------------------------------------
OldFormatPatternConverter::Report OldFormatPatternConverter::Convert(const QString &sourcePath,
                                                                      const QString &destPath,
                                                                      const QHash<QString, QString> &renameMap,
                                                                      const QString &newMeasurementsPath)
{
    Report report;

    if (QFileInfo(sourcePath) == QFileInfo(destPath))
    {
        report.errorMessage = QStringLiteral("Файл результата должен отличаться от исходного файла.");
        return report;
    }

    QFile inFile(sourcePath);
    if (!inFile.open(QIODevice::ReadOnly))
    {
        report.errorMessage = QStringLiteral("Не удалось открыть исходную выкройку для чтения.");
        return report;
    }

    VDomDocument doc;
    QString parseError;
    int errorLine = 0;
    int errorColumn = 0;
    const bool parsed = doc.setContent(&inFile, false, &parseError, &errorLine, &errorColumn);
    inFile.close();
    if (!parsed)
    {
        report.errorMessage = QStringLiteral("Файл не похож на выкройку (%1, строка %2).")
                                  .arg(parseError).arg(errorLine);
        return report;
    }

    QDomElement root = doc.documentElement();

    // 1. Every attribute of every element: renamed measurement names + old rint() syntax.
    // (QDomElement::elementsByTagName() has no wildcard, so walk the tree by hand.)
    QList<QDomElement> pending;
    pending.append(root);
    while (!pending.isEmpty())
    {
        QDomElement element = pending.takeLast();
        for (QDomNode child = element.firstChild(); !child.isNull(); child = child.nextSibling())
        {
            if (child.isElement())
            {
                pending.append(child.toElement());
            }
        }

        const QDomNamedNodeMap attributes = element.attributes();

        // Collect first: setAttribute() on the element being iterated would invalidate the map.
        QList<QPair<QString, QString> > updates;
        for (int a = 0; a < attributes.size(); ++a)
        {
            const QDomAttr attr = attributes.item(a).toAttr();
            const QString oldValue = attr.value();
            if (oldValue.isEmpty())
            {
                continue;
            }

            const QString renamed = OldFormatMeasurementsConverter::ApplyRenames(oldValue, renameMap);
            QString finalValue = renamed;
            if (renamed.contains(QStringLiteral("rint(")))
            {
                finalValue = OldFormatMeasurementsConverter::ConvertRintSyntax(renamed);
            }

            if (renamed != oldValue)
            {
                ++report.renamedAttributes;
            }
            if (finalValue != renamed)
            {
                ++report.rintConvertedCount;
            }
            if (finalValue != oldValue)
            {
                updates.append(qMakePair(attr.name(), finalValue));
            }
        }

        for (const QPair<QString, QString> &update : qAsConst(updates))
        {
            element.setAttribute(update.first, update.second);
        }
    }

    // 2. The reference to the measurements file.
    if (!newMeasurementsPath.isEmpty())
    {
        const QDomNodeList refs = root.elementsByTagName(QStringLiteral("measurements"));
        if (!refs.isEmpty())
        {
            QDomElement ref = refs.at(0).toElement();
            while (ref.hasChildNodes())
            {
                ref.removeChild(ref.firstChild());
            }
            const QString relative = QDir(QFileInfo(destPath).absolutePath()).relativeFilePath(newMeasurementsPath);
            ref.appendChild(doc.createTextNode(relative));
        }
    }

    // 3. A note that the file went through the converter (an XML comment: the pattern schema does not allow
    // unknown elements, comments are invisible to it).
    const QString conversionComment = QStringLiteral(" Converted from the old format (\"@\" + old rint() syntax) "
                                                       "by Seamly2D v%1 on %2. ")
                                           .arg(APP_VERSION_STR, QDate::currentDate().toString(Qt::ISODate));
    const QDomComment conversionInfo = doc.createComment(conversionComment);
    const QDomElement versionEl = root.firstChildElement(VDomDocument::TagVersion);
    if (!versionEl.isNull())
    {
        root.insertAfter(conversionInfo, versionEl);
    }
    else
    {
        root.insertBefore(conversionInfo, root.firstChild());
    }

    QString saveError;
    if (!doc.SaveDocument(destPath, saveError))
    {
        report.errorMessage = saveError.isEmpty() ? QStringLiteral("Не удалось сохранить итоговую выкройку.")
                                                  : saveError;
    }
    return report;
}
