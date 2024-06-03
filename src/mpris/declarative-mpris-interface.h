// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QObject>

class OrgFreedesktopDBusPropertiesInterface;
class OrgMprisMediaPlayer2PlayerInterface;

class MprisPlayer
{
public:
    MprisPlayer(const QString &serviceName, const QString &dbusObjectPath, const QDBusConnection &busConnection);
    MprisPlayer() = delete;

public:
    const QString &serviceName() const
    {
        return m_serviceName;
    }
    OrgFreedesktopDBusPropertiesInterface *propertiesInterface() const
    {
        return m_propertiesInterface.data();
    }
    OrgMprisMediaPlayer2PlayerInterface *mediaPlayer2PlayerInterface() const
    {
        return m_mediaPlayer2PlayerInterface.data();
    }

private:
    QString m_serviceName;
    QSharedPointer<OrgFreedesktopDBusPropertiesInterface> m_propertiesInterface;
    QSharedPointer<OrgMprisMediaPlayer2PlayerInterface> m_mediaPlayer2PlayerInterface;
};

class DeclarativeMprisInterface : public QObject
{
    Q_OBJECT

public:
    explicit DeclarativeMprisInterface(bool systemBus = false, QObject *parent = nullptr);
    ~DeclarativeMprisInterface() = default;

    void pauseAllOtherMprisPlayers(const QString &excludePlayerName);

private Q_SLOTS:
    void dbusServiceRegistered(const QString &serviceName);
    void dbusServiceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);
    void propertiesChanged(const QString &propertyInterface, const QVariantMap &properties);

private:
    QDBusServiceWatcher *m_serviceWatcher;
    void addPlayer(const QString &serviceName);
    void removePlayer(const QString &serviceName);
    QHash<QString, MprisPlayer> m_playerList;
    QDBusConnection m_dbus;
};
