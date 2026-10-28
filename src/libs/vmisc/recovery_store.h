//-----------------------------------------------------------------------------
//  @file   recovery_store.h
//  @date   23 Oct, 2026
//
//  @brief  Crash-safe storage of autosaved documents.
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#ifndef RECOVERY_STORE_H
#define RECOVERY_STORE_H

#include <QDateTime>
#include <QList>
#include <QLockFile>
#include <QScopedPointer>
#include <QString>
#include <QStringList>

/**
 * @brief Description of a document that was being edited when the program died.
 */
struct AbandonedSession
{
    QString     id;               ///< session id (base name of all session files)
    QString     app;              ///< "seamly2d" or "seamlyme"
    QString     originalPath;     ///< path of the document on disk; empty for a document never saved
    QString     title;            ///< human readable document name
    QDateTime   time;             ///< time of the newest autosave
    QStringList versions;         ///< absolute paths of autosave versions, newest first (may be empty)
};

/**
 * @brief One editing window = one recovery session.
 *
 * Autosaved copies live in a single central folder (never next to the user's file), so a program crash,
 * a read-only folder or a network drive cannot break them. The session keeps a lock file that the operating
 * system releases automatically when the process dies; another start of the program sees "no owner" and
 * knows the session was abandoned.
 *
 * Usage: start(); then for every autosave call beginVersion() -> write the document to the returned path ->
 * commitVersion(). Call discard() after the document was saved normally or closed on purpose.
 */
class RecoverySession
{
public:
    explicit RecoverySession(const QString &appName);
    ~RecoverySession();

    bool start();
    bool isActive() const;

    void setDocument(const QString &originalPath, const QString &title);

    /// Path to write the next autosave to. The extension is always "autosave".
    QString beginVersion();
    /// Call after the file from beginVersion() has been written successfully; rotates versions.
    bool    commitVersion();
    /// Removes every file of this session (normal exit or normal save).
    void    discard();

    QString id() const;

    static const int maxVersions = 3;

    static QString recoveryDirPath();
    static QList<AbandonedSession> findAbandoned(const QString &appName);
    static void removeSession(const QString &id);

private:
    Q_DISABLE_COPY(RecoverySession)

    QString filePath(const QString &suffix) const;
    bool    writeMeta() const;
    void    removeVersions();

    QString                    m_app;
    QString                    m_id;
    QString                    m_original;
    QString                    m_title;
    QScopedPointer<QLockFile>  m_lock;
};

#endif // RECOVERY_STORE_H
