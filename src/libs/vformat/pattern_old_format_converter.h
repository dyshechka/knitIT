/******************************************************************************
*   @file   pattern_old_format_converter.h
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
#ifndef PATTERN_OLD_FORMAT_CONVERTER_H
#define PATTERN_OLD_FORMAT_CONVERTER_H

#include <QHash>
#include <QString>

/// @brief One-shot conversion of an old-format pattern (.sm2d) to a new file. Never touches the source
/// file -- Convert() always writes to a different destination path.
///
/// A pattern refers to individual measurements by name inside its formulas. When the measurements file is
/// converted (OldFormatMeasurementsConverter: "@" removed from names, old rint() idiom replaced) the pattern
/// has to be rewritten with exactly the same renames, otherwise it cannot find its measurements any more.
/// So the pattern converter takes the rename map produced by the measurements converter and:
///  - replaces every whole-identifier occurrence of a renamed measurement name in every attribute of the
///    file (formulas of points, lines, arcs, curves, increments, ...);
///  - rewrites old-style "rint((<expr>)*10^n)/10^n" in those attributes to "rint(<expr>; n)";
///  - points the <measurements> element at the converted measurements file (relative path).
class OldFormatPatternConverter
{
public:
    struct Report
    {
        int renamedAttributes = 0;     ///< attribute values where at least one measurement name was renamed
        int rintConvertedCount = 0;    ///< attribute values with an updated rint() call
        QString errorMessage;          ///< non-empty on failure
    };

    /// @param newMeasurementsPath absolute path of the converted measurements file ("" -- leave the
    ///        <measurements> element as it is)
    static Report Convert(const QString &sourcePath, const QString &destPath,
                          const QHash<QString, QString> &renameMap, const QString &newMeasurementsPath);

    /// Returns the text of the <measurements> element of a pattern file ("" if there is none or the
    /// file can't be read).
    static QString MeasurementsReference(const QString &patternPath);
};

#endif // PATTERN_OLD_FORMAT_CONVERTER_H
