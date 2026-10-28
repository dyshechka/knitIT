//-----------------------------------------------------------------------------
//  @file   recovery_store.cpp
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

#include "recovery_store.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

namespace
{
const QString metaSuffix    = QStringLiteral("meta");
const QString lockSuffix    = QStringLiteral("lock");
const QString versionSuffix = QStringLiteral("autosave");

QString versionName(const QString &id, int number)
{
    return QStringLiteral("%1.v%2.%3").arg(id).arg(number).arg(versionSuffix);
}
}

//---------------------------------------------------------------------------------------------------------------------
QString RecoverySession::recoveryDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
           + QStringLiteral("/Seamly2D/recovery");
}

//---------------------------------------------------------------------------------------------------------------------
RecoverySession::RecoverySession(const QString &appName)
    : m_app(appName)
    , m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_original()
    , m_title()
    , m_lock()
{
}

//---------------------------------------------------------------------------------------------------------------------
RecoverySession::~RecoverySession()
{
    // Only release the lock. Files stay on disk on purpose: if we get here without discard() the
    // session is treated as abandoned and offered for recovery on the next start.
    if (m_lock && m_lock->isLocked())
    {
        m_lock->unlock();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString RecoverySession::id() const
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
QString RecoverySession::filePath(const QString &suffix) const
{
    return recoveryDirPath() + QLatin1Char('/') + m_id + QLatin1Char('.') + suffix;
}

//---------------------------------------------------------------------------------------------------------------------
bool RecoverySession::start()
{
    if (isActive())
    {
        return true;
    }

    if (!QDir().mkpath(recoveryDirPath()))
    {
        return false;
    }

    m_lock.reset(new QLockFile(filePath(lockSuffix)));
    // 0 = the lock is stale only when its owner process is dead, never because of its age
    m_lock->setStaleLockTime(0);
    if (!m_lock->tryLock(0))
    {
        m_lock.reset();
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool RecoverySession::isActive() const
{
    return m_lock && m_lock->isLocked();
}

//---------------------------------------------------------------------------------------------------------------------
void RecoverySession::setDocument(const QString &originalPath, const QString &title)
{
    m_original = originalPath;
    m_title    = title;
}

//---------------------------------------------------------------------------------------------------------------------
QString RecoverySession::beginVersion()
{
    if (!start())
    {
        return QString();
    }
    return filePath(QStringLiteral("new.") + versionSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
bool RecoverySession::commitVersion()
{
    const QString fresh = filePath(QStringLiteral("new.") + versionSuffix);
    if (!QFileInfo::exists(fresh))
    {
        return false;
    }

    const QString dir = recoveryDirPath();
    // shift: v(n-1) -> v(n), ..., v1 -> v2
    QFile::remove(dir + QLatin1Char('/') + versionName(m_id, maxVersions));
    for (int i = maxVersions - 1; i >= 1; --i)
    {
        const QString from = dir + QLatin1Char('/') + versionName(m_id, i);
        if (QFileInfo::exists(from))
        {
            QFile::rename(from, dir + QLatin1Char('/') + versionName(m_id, i + 1));
        }
    }

    if (!QFile::rename(fresh, dir + QLatin1Char('/') + versionName(m_id, 1)))
    {
        QFile::remove(fresh);
        return false;
    }
    return writeMeta();
}

//---------------------------------------------------------------------------------------------------------------------
bool RecoverySession::writeMeta() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("app"),      m_app);
    obj.insert(QStringLiteral("original"), m_original);
    obj.insert(QStringLiteral("title"),    m_title);
    obj.insert(QStringLiteral("time"),     QDateTime::currentDateTime().toString(Qt::ISODate));

    QSaveFile file(filePath(metaSuffix));
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    return file.commit();
}

//---------------------------------------------------------------------------------------------------------------------
void RecoverySession::removeVersions()
{
    const QString dir = recoveryDirPath();
    for (int i = 1; i <= maxVersions; ++i)
    {
        QFile::remove(dir + QLatin1Char('/') + versionName(m_id, i));
    }
    QFile::remove(filePath(QStringLiteral("new.") + versionSuffix));
    QFile::remove(filePath(metaSuffix));
}

//---------------------------------------------------------------------------------------------------------------------
void RecoverySession::discard()
{
    removeVersions();
    if (m_lock && m_lock->isLocked())
    {
        m_lock->unlock(); // also removes the lock file
    }
}

//---------------------------------------------------------------------------------------------------------------------
void RecoverySession::removeSession(const QString &id)
{
    const QString dir = recoveryDirPath();
    for (int i = 1; i <= maxVersions; ++i)
    {
        QFile::remove(dir + QLatin1Char('/') + versionName(id, i));
    }
    QFile::remove(dir + QLatin1Char('/') + id + QStringLiteral(".new.") + versionSuffix);
    QFile::remove(dir + QLatin1Char('/') + id + QLatin1Char('.') + metaSuffix);
    QFile::remove(dir + QLatin1Char('/') + id + QLatin1Char('.') + lockSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
QList<AbandonedSession> RecoverySession::findAbandoned(const QString &appName)
{
    QList<AbandonedSession> result;

    const QDir dir(recoveryDirPath());
    const QStringList metas = dir.entryList(QStringList() << QStringLiteral("*.") + metaSuffix, QDir::Files);
    for (const QString &metaName : metas)
    {
        const QString id = QFileInfo(metaName).completeBaseName();

        // Is the owner still alive? tryLock succeeds when the lock is free or its owner process is dead.
        QLockFile lock(dir.filePath(id + QLatin1Char('.') + lockSuffix));
        lock.setStaleLockTime(0);
        if (!lock.tryLock(0))
        {
            continue; // another window of the program is working on it right now
        }

        QFile metaFile(dir.filePath(metaName));
        if (!metaFile.open(QIODevice::ReadOnly))
        {
            continue;
        }
        const QJsonObject obj = QJsonDocument::fromJson(metaFile.readAll()).object();
        metaFile.close();
        if (obj.value(QStringLiteral("app")).toString() != appName)
        {
            continue;
        }

        AbandonedSession session;
        session.id           = id;
        session.app          = appName;
        session.originalPath = obj.value(QStringLiteral("original")).toString();
        session.title        = obj.value(QStringLiteral("title")).toString();
        session.time         = QDateTime::fromString(obj.value(QStringLiteral("time")).toString(), Qt::ISODate);
        for (int i = 1; i <= maxVersions; ++i)
        {
            const QString path = dir.filePath(versionName(id, i));
            if (QFileInfo::exists(path))
            {
                session.versions.append(path);
            }
        }
        if (session.versions.isEmpty())
        {
            // nothing to restore: just clean up the leftovers silently
            lock.unlock();
            removeSession(id);
            continue;
        }
        result.append(session);
    }
    return result;
}
