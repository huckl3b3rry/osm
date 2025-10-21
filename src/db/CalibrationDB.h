/**
 *  OSM - Calibration DB
 *  // V1.0: Bolt Migration - Added SQLite
 *
 *  Minimal local-first calibration storage built on Qt SQLite.
 *  Keeps schema simple and robust; no external dependencies.
 */
#ifndef CALIBRATIONDB_H
#define CALIBRATIONDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDir>

class CalibrationDB : public QObject
{
    Q_OBJECT

public:
    explicit CalibrationDB(QObject *parent = nullptr);
    ~CalibrationDB() override;

    // Open or create a DB file at path. Creates parent folders if needed.
    // Returns true on success. Logs detailed errors otherwise.
    bool openOrCreate(const QString &dbPath);

    // Simple write to verify DB is usable. Returns inserted row id or -1.
    qint64 insertTestSession(const QString &note = QString("test session"));

    QString databasePath() const { return m_dbPath; }

    // Suggested default location under app data folder.
    static QString defaultProjectDbPath(const QString &projectName = QString("DefaultProject"));

    // Expose connection handle for advanced callers (implicitly shared).
    QSqlDatabase db() const { return m_db; }

private:
    bool ensureSchema();
    static bool execOrLog(QSqlQuery &q);

    QString m_connectionName;
    QString m_dbPath;
    QSqlDatabase m_db;
};

#endif // CALIBRATIONDB_H
