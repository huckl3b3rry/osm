/**
 *  OSM - Session Manager Stub
 *  // V1.1: Sweep for calibration - session scaffold
 */
#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QString>
#include <memory>

class CalibrationDB;

struct MeasurementTask {
    int speakerId;
    QString note;
};

class SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() override;

    // Initialize DB for project; returns true if ready for use.
    bool initProject(const QString &projectName);

    // Minimal metadata for MVP.
    qint64 startSession(const QString &roomName, int speakerCount);
    void stopSession();

    void queueMeasurements(const QVector<MeasurementTask> &tasks);

    // Simple stub analysis; returns a human-readable recommendation.
    static QString analyzeStub(const QByteArray &recording);

    CalibrationDB *db() const { return m_db.get(); }
    qint64 currentSessionId() const { return m_currentSessionId; }

signals:
    void sessionStarted(qint64 sessionId);
    void sessionStopped();

private:
    std::unique_ptr<CalibrationDB> m_db;
    qint64 m_currentSessionId;
    QVector<MeasurementTask> m_queue;
};

#endif // SESSIONMANAGER_H
