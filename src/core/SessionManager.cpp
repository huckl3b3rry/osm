/**
 *  OSM - Session Manager Stub
 *  // V1.0: Bolt Migration - basic local session handling
 */
#include "SessionManager.h"
#include "src/db/CalibrationDB.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

SessionManager::SessionManager(QObject *parent)
    : QObject(parent), m_db(new CalibrationDB(this)), m_currentSessionId(-1)
{
}

SessionManager::~SessionManager() = default;

bool SessionManager::initProject(const QString &projectName)
{
    const auto dbPath = CalibrationDB::defaultProjectDbPath(projectName);
    if (dbPath.isEmpty()) {
        qDebug() << "SessionManager: no writable path for DB";
        return false;
    }
    return m_db->openOrCreate(dbPath);
}

qint64 SessionManager::startSession(const QString &roomName, int speakerCount)
{
    if (!m_db) return -1;

    // Insert a room (optional fields left null)
    QSqlQuery qRoom(m_db->db());
    qRoom.prepare("INSERT INTO rooms(name) VALUES(?)");
    qRoom.addBindValue(roomName);
    if (!qRoom.exec()) {
        qDebug() << "SessionManager: room insert failed" << qRoom.lastError();
    }
    const auto roomIdVariant = qRoom.lastInsertId();
    const auto roomId = roomIdVariant.isValid() ? roomIdVariant.toLongLong() : 0;

    // Insert session
    QSqlQuery q(m_db->db());
    q.prepare("INSERT INTO sessions(name, created_at, room_id, note) VALUES(?, ?, ?, ?)" );
    q.addBindValue(QString("Calib %1 spk").arg(speakerCount));
    q.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    q.addBindValue(roomId);
    q.addBindValue(QString("Room:%1 Speakers:%2").arg(roomName).arg(speakerCount));

    if (!q.exec()) {
        qDebug() << "SessionManager: session insert failed" << q.lastError();
        return -1;
    }

    m_currentSessionId = q.lastInsertId().toLongLong();
    emit sessionStarted(m_currentSessionId);
    return m_currentSessionId;
}

void SessionManager::stopSession()
{
    m_currentSessionId = -1;
    emit sessionStopped();
}

void SessionManager::queueMeasurements(const QVector<MeasurementTask> &tasks)
{
    m_queue = tasks;
}

QString SessionManager::analyzeStub(const QByteArray &recording)
{
    Q_UNUSED(recording);
    // Placeholder; in future compute FFT and basic EQ suggestion
    return QStringLiteral("EQ rec: Boost 100Hz +3dB");
}
