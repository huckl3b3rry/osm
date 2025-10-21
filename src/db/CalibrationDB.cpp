/**
 *  OSM - Calibration DB
 *  // V1.0: Bolt Migration - Added SQLite
 *
 *  Creates per-project SQLite stores with core calibration tables.
 *  Errors are logged via QDebug but never throw.
 */
#include "CalibrationDB.h"
#include <QFileInfo>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QApplication>
#include <QDebug>

CalibrationDB::CalibrationDB(QObject *parent)
    : QObject(parent)
{
    m_connectionName = QString("calibration_%1").arg(reinterpret_cast<quintptr>(this));
}

CalibrationDB::~CalibrationDB()
{
    if (m_db.isValid()) {
        m_db.close();
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

QString CalibrationDB::defaultProjectDbPath(const QString &projectName)
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) {
        base = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    }
    if (base.isEmpty()) {
        qDebug() << "CalibrationDB: No writable app data path";
        return QString();
    }
    if (base.indexOf(QApplication::applicationName()) == -1) {
        base += "/" + QApplication::applicationName();
    }
    QDir().mkpath(base + "/projects");
    return base + "/projects/" + projectName + ".db";
}

bool CalibrationDB::openOrCreate(const QString &dbPath)
{
    m_dbPath = dbPath;
    QFileInfo fi(dbPath);
    if (!fi.dir().exists()) {
        if (!QDir().mkpath(fi.dir().absolutePath())) {
            qDebug() << "CalibrationDB: Failed to create dir" << fi.dir().absolutePath();
            return false;
        }
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "CalibrationDB: open failed" << m_db.lastError();
        return false;
    }

    // Handle locked DB (busy). SQLite returns errors at exec time; we keep it simple.
    return ensureSchema();
}

bool CalibrationDB::ensureSchema()
{
    // Schema: sessions, measurements, speakers, rooms, analysis_results
    // Keep columns minimal and forward-compatible.
    QSqlQuery q(m_db);

    q.prepare("PRAGMA journal_mode=WAL");
    execOrLog(q);

    q.prepare("CREATE TABLE IF NOT EXISTS sessions (\n"
              "  id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
              "  name TEXT NOT NULL,\n"
              "  created_at TEXT NOT NULL,\n"
              "  room_id INTEGER,\n"
              "  note TEXT\n"
              ")");
    if (!execOrLog(q)) return false;

    q.prepare("CREATE TABLE IF NOT EXISTS rooms (\n"
              "  id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
              "  name TEXT,\n"
              "  width REAL,\n"
              "  height REAL,\n"
              "  depth REAL\n"
              ")");
    if (!execOrLog(q)) return false;

    q.prepare("CREATE TABLE IF NOT EXISTS speakers (\n"
              "  id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
              "  name TEXT,\n"
              "  position_x REAL,\n"
              "  position_y REAL,\n"
              "  position_z REAL\n"
              ")");
    if (!execOrLog(q)) return false;

    q.prepare("CREATE TABLE IF NOT EXISTS measurements (\n"
              "  id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
              "  session_id INTEGER NOT NULL,\n"
              "  speaker_id INTEGER,\n"
              "  path TEXT,\n"
              "  taken_at TEXT NOT NULL,\n"
              "  note TEXT,\n"
              "  FOREIGN KEY(session_id) REFERENCES sessions(id)\n"
              ")");
    if (!execOrLog(q)) return false;

    q.prepare("CREATE TABLE IF NOT EXISTS analysis_results (\n"
              "  id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
              "  measurement_id INTEGER NOT NULL,\n"
              "  key TEXT NOT NULL,\n"
              "  value TEXT,\n"
              "  created_at TEXT NOT NULL,\n"
              "  FOREIGN KEY(measurement_id) REFERENCES measurements(id)\n"
              ")");
    if (!execOrLog(q)) return false;

    return true;
}

qint64 CalibrationDB::insertTestSession(const QString &note)
{
    if (!m_db.isOpen()) return -1;
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO sessions(name, created_at, note) VALUES(?, ?, ?)");
    q.addBindValue(QStringLiteral("Test Session"));
    q.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    q.addBindValue(note);
    if (!execOrLog(q)) return -1;
    return q.lastInsertId().toLongLong();
}

bool CalibrationDB::execOrLog(QSqlQuery &q)
{
    if (!q.exec()) {
        qDebug() << "CalibrationDB SQL error:" << q.lastError() << "SQL:" << q.lastQuery();
        return false;
    }
    return true;
}
