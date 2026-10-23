/******************************************************************************
 *   @file   tmainwindow.h
 **  @author Douglas S Caskey
 **  @date   25 Jan, 2024
 **
 **  @brief
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

 /************************************************************************
 **
 **  @file   tmainwindow.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include <QAction>
#include <QColor>
#include <QFont>
#include <QHash>
#include <QMenu>
#include <QMap>
#include <QSet>
#include <QToolButton>
#include <QLineEdit>
#include <QTimer>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QVector>

#include "../vmisc/def.h"
#include "../vmisc/vlockguard.h"
#include "../vmisc/vtablesearch.h"
#include "../vpatterndb/variables/measurement_variable.h"
#include "../vpatterndb/variables/vinternalvariable.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "dialogs/me_shortcuts_dialog.h"

namespace Ui
{
    class TMainWindow;
}

class QLabel;
class MeShortcutsDialog;
class MeasurementDoc;
class VContainer;

// A plain number the formula parser can use as a variable ("X" and "CM" in the correction formula).
class LoopScalarVariable : public VInternalVariable
{
public:
    explicit LoopScalarVariable(qreal value = 0) { SetValue(value); }
    void Set(qreal value) { SetValue(value); }
};

class TMainWindow : public VAbstractMainWindow
{
    Q_OBJECT

public:
    explicit            TMainWindow(QWidget *parent = nullptr);
    virtual            ~TMainWindow() override;

    QString             CurrentFile() const;

    void                RetranslateTable();

    void                SetBaseMHeight(int height);
    void                SetBaseMSize(int size);
    void                setPUnit(Unit unit);

    bool                LoadFile(const QString &path);

public slots:
    virtual void        setStatusMessage(const QString &toolTip) override;
    virtual void        zoomToSelected() override;
    virtual void        updateGroups() override;

protected:
    virtual void        closeEvent(QCloseEvent *event) override;
    virtual void        changeEvent(QEvent* event) override;
    virtual void        showEvent(QShowEvent *event) override;
    virtual bool        eventFilter(QObject *object, QEvent *event) override;
    virtual void        exportToCSVData(const QString &fileName, const DialogExportToCSV &dialog) final;
    void                handleExportToCSV();

private slots:
    void                FileNew();
    void                OpenIndividual();
    void                OpenMultisize();
    void                OpenTemplate();
    void                CreateFromExisting();
    void                ConvertOldFormat();
    //void                handleBodyScanner1();
    void                handleBodyScanner2();
    void                Preferences();
    void                initToolBarStyles();

    void                print();
    void                printPages(QPrinter *printer);

    bool                FileSave();
    bool                FileSaveAs();
    void                AboutToShowWindowMenu();
    void                ShowWindow() const;

#if defined(Q_OS_MAC)
    void                AboutToShowDockMenu();
    void                OpenAt(QAction *where);
#endif //defined(Q_OS_MAC)

    void                SaveGivenName();
    void                SaveFamilyName();
    void                SaveEmail();
    void                SaveGender(int index);
    void                SaveBirthDate(const QDate & date);
    void                SaveNotes();
    void                SavePMSystem(int index);

    void                Remove();
    void                MoveTop();
    void                MoveUp();
    void                MoveDown();
    void                MoveBottom();
    void                Fx();

    void                AddCustom();
    void                AddKnown();
    void                ImportFromPattern();

    void                ChangedSize(int index);
    void                ChangedHeight(int index);

    void                ShowMData();

    void                SaveMName(const QString &text);
    void                SaveMValue();
    void                CommitMValue();
    void                SaveMBaseValue(double value);
    void                SaveMSizeIncrease(double value);
    void                SaveMHeightIncrease(double value);
    void                SaveMDescription();
    void                SaveMFullName();
    void                SaveMIsSection(bool checked);
    void                SaveMIsLoopCalc(bool checked);
    void                ApplyLoopFilter();
    void                SetLoopCalcForRow(int row, bool checked, bool fromTable);
    void                OnTableItemChanged(QTableWidgetItem *item);
    void                SaveMPattern();

    void                patternUnitsChanged(int index);

private:
    Q_DISABLE_COPY(TMainWindow)
    Ui::TMainWindow    *ui;
    MeasurementDoc     *individualMeasurements;
    VContainer         *data;
    Unit                mUnit;
    Unit                pUnit;
    MeasurementsType    mType;
    qreal               currentSize;
    qreal               currentHeight;
    QString             curFile;
    QComboBox          *gradationHeights;
    QComboBox          *gradationSizes;
    QComboBox          *comboBoxUnits;

    std::shared_ptr<VLockGuard<char>> lock;
    QSharedPointer<VTableSearch>      m_search;
    QLabel             *labelGradationHeights;
    QLabel             *labelGradationSizes;
    QLabel             *labelPatternUnit;
    QLineEdit          *editGaugeStitches = nullptr;
    QLineEdit          *editGaugeRows = nullptr;
    QTimer             *m_refreshTimer = nullptr;   // the table is rebuilt a moment after typing stops

    // Kept for the time of one rebuild of the table (see ClearCalcCaches()): the reason a value can't be
    // calculated, and the variables the correction formulas are calculated with.
    mutable QHash<QString, QString>                           m_valueErrorCache;
    mutable QHash<QString, QSharedPointer<VInternalVariable> > m_loopVariables;
    mutable QSharedPointer<LoopScalarVariable>                m_loopX;
    mutable QSharedPointer<LoopScalarVariable>                m_loopCm;
    QLabel             *labelGaugeStitches = nullptr;
    QList<QAction *>    m_gaugeActions;
    QAction            *actionDockDiagram;
    bool                dockDiagramVisible;
    bool                isInitialized;
    bool                m_isReadOnly;
    enum { MaxRecentFiles = 5 };
    QAction            *recentFileActs[MaxRecentFiles];
    QAction            *separatorAct;
    QVector<QObject *>  hackedWidgets;
    QString             m_currentSvgPath;   // Stores the current "://diagrams/..." path
    QString             m_currentNumber;    // Stores the current measurement number (e.g. "L13")
    QString             m_currentName;      // Stores the translated name text
    QString             m_currentDescription;

    // Row-highlighting support for the measurements table -- see RefreshTable().
    // m_usedByMeasurement is a reverse formula-dependency index (name -> names of measurements
    // whose formula references it), rebuilt every time the table is refreshed.
    // m_affectedMeasurements holds the names of measurements whose row should show the "check
    // me, something I depend on just changed" (pink) highlight -- populated by
    // MarkMeasurementSaved(), cleared once that row is viewed (ShowNewMData()) or its own
    // formula is (re)saved.
    // m_formulaDrafts holds unsaved, currently-invalid formula text by measurement name (orange
    // highlight) -- see CommitMValue()/ShowNewMData().
    QMap<QString, QStringList> m_usedByMeasurement;
    QSet<QString>              m_affectedMeasurements;
    QMap<QString, QString>     m_formulaDrafts;

    // Name of the measurement currently reflected in the Details panel (plainTextEditFormula
    // included), or empty when no row is shown. A name rather than a row index because rows can
    // shift (add/remove) between when this is recorded and when it's used. Needed because a
    // mouse click on a different table row can update ui->tableWidget->currentRow() to the NEW
    // row before plainTextEditFormula's FocusOut (and so CommitMValue(), see eventFilter()) is
    // actually delivered -- by then, committing against currentRow() would target the wrong row
    // entirely, silently discarding whatever was typed for the row she's leaving. See
    // ShowNewMData().
    QString                     m_editingMeasurementName;

    // "View" menu: which table columns are shown (the same set goes to the print preview and the
    // CSV export), the row filter, text size, row striping and panels. Kept in the settings.
    QHash<int, QAction *>       m_columnActions;
    QAction                    *m_filterActions[3] = {nullptr, nullptr, nullptr};
    QAction                    *m_textSizeActions[3] = {nullptr, nullptr, nullptr};
    QAction                    *m_alternateRowsAction = nullptr;
    QAction                    *m_detailsAction = nullptr;
    QAction                    *m_toolbarAction = nullptr;
    QMenu                      *m_columnsMenu = nullptr;
    QToolButton                *m_columnsButton = nullptr;
    QWidget                    *m_cornerButton = nullptr;
    QToolButton                *m_toolbarButton = nullptr;
    QMenu                      *m_showMenu = nullptr;
    QFont                       m_baseTableFont;
    bool                        m_alternateRows = true;

    void                SetupMenu();
    void                SetupViewMenu();
    void                ApplyColumnVisibility();
    void                ApplyTextSize();
    void                ResetView();
    void                ShowHeaderMenu(const QPoint &position);
    bool                ColumnShown(int column) const;
    QVector<int>        ExportColumns() const;
    void                InitWindow();
    void                initializeTable();
    void                RetranslateTableHeaders();
    void                SetDecimals();
    void                initUnits();
    void                setCurrentPatternUnits();
    void                InitComboBoxUnits();
    void                InitGender(QComboBox *gender);

    void                ShowNewMData(bool fresh);
    void                ShowUnits();
    void                ShowHeaderUnits(QTableWidget *table, int column, const QString &unit);
    void                UpdateRecentFileActions();

    void                MeasurementsWasSaved(bool saved);
    void                SetCurrentFile(const QString &fileName);
    bool                SaveMeasurements(const QString &fileName, QString &error);

    bool                CanReplaceCurrentWindow() const;
    bool                MaybeSave();

    QTableWidgetItem   *AddCell(const QString &text, int row, int column, int aligment, bool ok = true);

    // One row of the exported/printed measurements table (Individual files only). Built from the
    // measurement data itself, not read back from the on-screen table -- see CollectExportRows().
    // The stitch/row result of a loop calculation (see ComputeLoopResult()).
    struct LoopResult
    {
        enum Kind {None = 0, Stitch, Row, Cm};
        Kind    kind = None;
        QString text;      // what the cell shows ("86 st.", "120 rw.", "12.5 cm", or a dash / nothing)
        QString tooltip;
        QString numberText; // just the number ("42"), to put into "own number"
        bool    error = false;
    };

    struct ExportRow
    {
        bool    loopCalc = false;
        QString pattern;
        QString name;
        QString fullName;
        QString formula;
        QString calculation;
        QString value;
        QString description;
        bool    isSection = false;
        QString stitchRow;          // result in stitches/rows/cm
        int     stitchKind = 0;     // LoopResult::Kind
        bool    stitchError = false;
        QString adjustment;
        bool    hasError = false;   // the value cell holds an error text instead of a number
        QColor  highlight;          // row color from the table (orange/pink/blue); invalid = plain row
    };

    // "Show by loop calculation" filter next to the search field: 0 -- everything, 1 -- section
    // dividers and loop calculations only, 2 -- section dividers and everything that is not a
    // loop calculation. Shared by the table, the CSV export and the print preview.
    int                 LoopFilterMode() const;
    bool                LoopFilterAllows(const QSharedPointer<MeasurementVariable> &meash) const;
    void                SelectVisibleNear(int row, bool notify);
    void                ResetLoopFilter();

    QStringList         ExportHeaders() const;
    QVector<ExportRow>  CollectExportRows() const;
    QColor              TableRowHighlight(const QString &internalName) const;
    LoopResult          ComputeLoopResult(const QSharedPointer<MeasurementVariable> &meash) const;
    LoopResult::Kind    LoopKindOf(const QSharedPointer<MeasurementVariable> &meash) const;
    void                SaveMAdjustment();
    void                AdjustModeActivated(int index);
    void                ResultUnitActivated(int index);
    void                SaveGauge();
    void                UpdateGaugeStyle();
    void                FxAdjustment();
    void                CapValueColumn();
    void                ApplyDefaultColumnWidths(int onlyColumn = -1);
    void                ApplyHeaderFont();
    void                LoadGauge();
    void                StoreAdjustment(const QSharedPointer<MeasurementVariable> &meash, const QString &adjustment,
                                        const QString &resultUnit);
    void                UpdateLoopDetails(const QSharedPointer<MeasurementVariable> &meash);
    void                UpdateLoopResultLabel(const QSharedPointer<MeasurementVariable> &meash);
    void                SetLoopDetailsVisible(bool visible);
    QSharedPointer<MeasurementVariable> CurrentMeasurement() const;
    void                ScheduleRefresh();
    void                FlushPendingRefresh();
    void                DeferredRefresh();
    void                ClearCalcCaches() const;
    static QString      AdjustModeOf(const QString &adjustment);
    static QString      AdjustModeExpression(const QString &mode);
    QString             AdjustModeLabel(const QString &mode) const;
    QString             AdjustmentDisplayText(const QSharedPointer<MeasurementVariable> &meash) const;
    QString             ValueErrorText(const QSharedPointer<MeasurementVariable> &meash) const;
    QString             ValueErrorTextCompute(const QSharedPointer<MeasurementVariable> &meash) const;
    QString             BuildCalculation(const QSharedPointer<MeasurementVariable> &meash,
                                         const QMap<QString, QSharedPointer<MeasurementVariable> > &table) const;

    void                SetRowHighlight(int row, const QColor &color);
    void                MarkMeasurementSaved(const QString &name);
    void                CommitMValueFor(const QString &measurementName, bool restoreSelection);

    Q_REQUIRED_RESULT QComboBox *SetGradationList(QLabel *label, const QStringList &list);

    void                SetDefaultHeight(int value);
    void                SetDefaultSize(int value);

    void                RefreshData(bool freshCall = false);
    void                RefreshTable(bool freshCall = false);

    QString             GetCustomName() const;
    void                RegisterNewKnitMeasurements();

    bool                EvalFormula(const QString &formula, bool fromUser, VContainer *data, QLabel *label);
    // Does `internalFormula` (a formula in internal/untranslated form, e.g. meash->GetFormula())
    // refer to a measurement that's marked as a section divider (checkBoxIsSection/IsSection())?
    // A divider has no real value -- it's purely organizational -- so using it in a calculation
    // is always a mistake, even though the Calculator has no trouble evaluating it (a divider
    // row is still stored as a variable with a numeric value -- see
    // MeasurementDoc::readMeasurements()). On a true result, *sectionName (if given) is set to
    // the divider's user-facing (translated) name for use in an error message. See RefreshTable().
    bool                FormulaReferencesSection(const QString &internalFormula, QString *sectionName = nullptr) const;
    QString             getMeasurementNumber(const QString &name);
    void                ShowMDiagram(QSharedPointer<MeasurementVariable> meash);

    void                Open(const QString &pathTo, const QString &filter);
    void                UpdatePadlock(bool ro);
    void                MeasurementGUI();
    void                Controls();
    void                MFields(bool enabled);
    void                UpdateWindowTitle();

    void                ReadSettings();
    void                WriteSettings();

    QStringList         FilterMeasurements(const QStringList &mNew, const QStringList &mFilter);

    void                updatePatternUnit();

    bool                LoadFromExistingFile(const QString &path);

    void                CreateWindowMenu(QMenu *menu);

    bool                IgnoreLocking(int error, const QString &path);

    template <class T>
    void                HackWidget(T **widget);
    void                copyToClipboard();
    void                renderScaledDiagram();
};

#endif // TMAINWINDOW_H
