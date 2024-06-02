// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: BSD-3-Clause

#include "declarative-mpris-interface.h"
#include <QDBusConnection>
#include <QDebug>

#include "dbuspropertiesinterface.h"
#include "mprisinterface.h"
#include "mprisplayerinterface.h"

const auto mprisMediaPlayer2Interace = QLatin1String("org.mpris.MediaPlayer2");
const auto mprisMediaPlayerObjectPath = QLatin1String("/org/mpris/MediaPlayer2");
const auto dBusPropertiesInterface = QLatin1String("org.freedesktop.DBus.Properties");
const auto dBusPropertiesChangedSignal = QLatin1String("PropertiesChanged");
const auto mprisMediaPlayerPlaybackStatus = QLatin1String("PlaybackStatus");
const auto mprisMediaPlayerPlaybackStatusPlaying = QLatin1String("Playing");

MprisPlayer::MprisPlayer(const QString &serviceName, const QString &dbusObjectPath, const QDBusConnection &busConnection)
    : m_serviceName(serviceName)
    , m_propertiesInterface(new OrgFreedesktopDBusPropertiesInterface(serviceName, dbusObjectPath, busConnection))
    , m_mediaPlayer2PlayerInterface(new OrgMprisMediaPlayer2PlayerInterface(serviceName, dbusObjectPath, busConnection))
{
    m_mediaPlayer2PlayerInterface->setTimeout(300); // default D-Bus timeout is 25 seconds
}

DeclarativeMprisInterface::DeclarativeMprisInterface(QObject *parent)
{
    m_serviceWatcher = new QDBusServiceWatcher(mprisMediaPlayer2Interace + QLatin1String("*"), QDBusConnection::sessionBus());

    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &DeclarativeMprisInterface::dbusServiceRegistered);
    // TODO: since QDBusConnectionInterface::serviceOwnerChanged is deprecated, query org.freedesktop.DBus directly
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &DeclarativeMprisInterface::dbusServiceOwnerChanged);

    // Init existing interfaces
    const QStringList services = QDBusConnection::sessionBus().interface()->registeredServiceNames().value();
    for (const QString &service : services) {
        if (service.startsWith(mprisMediaPlayer2Interace)) {
            // The owners content doesn't matter, it just needs to be empty/non-empty to be parsed as new one
            dbusServiceOwnerChanged(service, QLatin1String(""), QStringLiteral("non-empty"));
        }
    }
}

void DeclarativeMprisInterface::pauseAllOtherMprisPlayers(const QString &excludePlayerName)
{
    qDebug() << Q_FUNC_INFO << excludePlayerName;

    for (auto i = m_playerList.cbegin(), end = m_playerList.cend(); i != end; ++i) {
        const QString playerName = i.key();
        if (playerName == excludePlayerName) {
            continue;
        }
        const MprisPlayer player = i.value();
        player.mediaPlayer2PlayerInterface()->Pause();
    }
}

void DeclarativeMprisInterface::dbusServiceRegistered(const QString &serviceName)
{
    qDebug() << Q_FUNC_INFO << serviceName;
}

void DeclarativeMprisInterface::dbusServiceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    if (!oldOwner.isEmpty()) {
        removePlayer(serviceName);
    }

    if (!newOwner.isEmpty()) {
        addPlayer(serviceName);
    }
}

void DeclarativeMprisInterface::propertiesChanged(const QString &propertyInterface, const QVariantMap &properties)
{
    OrgFreedesktopDBusPropertiesInterface *propertiesInterface = (OrgFreedesktopDBusPropertiesInterface *)sender();
    const auto end = m_playerList.constEnd();
    const auto it = std::find_if(m_playerList.constBegin(), end, [propertiesInterface](const MprisPlayer &player) {
        return (player.propertiesInterface() == propertiesInterface);
    });
    if (it == end) {
        qDebug() << "PropertiesChanged signal received for no longer tracked service" << propertiesInterface->service();
        return;
    }

    const QString &playerName = it.key();

    if (properties.contains(mprisMediaPlayerPlaybackStatus)) {
        bool playing = (properties[mprisMediaPlayerPlaybackStatus].toString() == mprisMediaPlayerPlaybackStatusPlaying);
        if (playing) {
            pauseAllOtherMprisPlayers(playerName);
        }
    }
}

void DeclarativeMprisInterface::addPlayer(const QString &serviceName)
{
    OrgMprisMediaPlayer2Interface iface(serviceName, mprisMediaPlayerObjectPath, QDBusConnection::sessionBus());
    QString identity = iface.identity();

    if (identity.isEmpty()) {
        identity = serviceName.mid(sizeof(mprisMediaPlayer2Interace));
    }

    QString uniqueName = identity;
    for (int i = 2; m_playerList.contains(uniqueName); ++i) {
        uniqueName = identity + QLatin1String(" [") + QString::number(i) + QLatin1Char(']');
    }

    qDebug() << Q_FUNC_INFO << uniqueName << serviceName;

    MprisPlayer player(serviceName, mprisMediaPlayerObjectPath, QDBusConnection::sessionBus());

    m_playerList.insert(uniqueName, player);

    connect(player.propertiesInterface(), &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this, &DeclarativeMprisInterface::propertiesChanged);
}

void DeclarativeMprisInterface::removePlayer(const QString &serviceName)
{
    const auto end = m_playerList.end();
    const auto it = std::find_if(m_playerList.begin(), end, [serviceName](const MprisPlayer &player) {
        return (player.serviceName() == serviceName);
    });
    if (it == end) {
        qDebug() << "Could not find player for serviceName" << serviceName;
        return;
    }

    const QString &playerName = it.key();
    qDebug() << "Mpris removePlayer" << serviceName << "->" << playerName;

    m_playerList.erase(it);
}
