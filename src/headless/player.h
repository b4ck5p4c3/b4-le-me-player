// SPDX-License-Identifier: BSD-3-Clause
#pragma once
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <mpv/client.h>

class Player final : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = nullptr);
    ~Player() override;
    QString initialize(const QString &audioOutput = {});
    Q_INVOKABLE QString execute(const QJsonObject &command);
    Q_PROPERTY(QJsonObject playbackState READ state NOTIFY stateChanged)
    QJsonObject state() const;
Q_SIGNALS:
    void stateChanged();
    void commandFinished(const QString &id, const QString &error);

private:
    static void wakeup(void *context);
    void drainEvents();
    mpv_handle *m_handle = nullptr;
    bool m_initialized = false;
    quint64 m_nextRequest = 0;
    QHash<quint64, QString> m_requests;
    QString m_pendingPlayback;
    quint64 m_pendingLoad = 0;
    quint64 m_pendingStop = 0;
    qint64 m_entryId = -1;
    QJsonObject m_state{{"status", "idle"}};
};
