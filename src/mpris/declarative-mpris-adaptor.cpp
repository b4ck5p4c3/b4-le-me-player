// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: BSD-3-Clause

#include "declarative-mpris-adaptor.h"
#include <QDBusMessage>
#include <QDebug>
#include <QMetaClassInfo>

const auto mprisMediaPlayerObjectPath = QLatin1String("/org/mpris/MediaPlayer2");
const auto dBusPropertiesInterface = QLatin1String("org.freedesktop.DBus.Properties");
const auto dBusPropertiesChangedSignal = QLatin1String("PropertiesChanged");

const auto mprisMediaPlayerPlaybackStatus = QLatin1String("PlaybackStatus");

DeclarativeMprisAdaptor::DeclarativeMprisAdaptor(bool systemBus, QObject *parent)
    : m_dbus(systemBus ? QDBusConnection::systemBus() : QDBusConnection::sessionBus())
{
    connect(this, &DeclarativeMprisAdaptor::playbackStatusChanged, this, [this]() {
        signalPropertyChange(mprisMediaPlayerPlaybackStatus, m_PlaybackStatus);
    });
}

bool DeclarativeMprisAdaptor::canControl() const
{
    return m_CanControl;
}

bool DeclarativeMprisAdaptor::canGoNext() const
{
    return m_CanGoNext;
}

bool DeclarativeMprisAdaptor::canGoPrevious() const
{
    return m_CanGoPrevious;
}

bool DeclarativeMprisAdaptor::canPause() const
{
    return m_CanPause;
}

bool DeclarativeMprisAdaptor::canPlay() const
{
    return m_CanPlay;
}

bool DeclarativeMprisAdaptor::canSeek() const
{
    return m_CanSeek;
}

QString DeclarativeMprisAdaptor::loopStatus() const
{
    return m_LoopStatus;
}

void DeclarativeMprisAdaptor::setLoopStatus(const QString &value)
{
    if (m_LoopStatus != value) {
        m_LoopStatus = value;
        Q_EMIT loopStatusChanged();
    }
}

double DeclarativeMprisAdaptor::maximumRate() const
{
    return m_MaximumRate;
}

QVariantMap DeclarativeMprisAdaptor::metadata() const
{
    return m_Metadata;
}

double DeclarativeMprisAdaptor::minimumRate() const
{
    return m_MinimumRate;
}

QString DeclarativeMprisAdaptor::playbackStatus() const
{
    return m_PlaybackStatus;
}

void DeclarativeMprisAdaptor::setPlaybackStatus(const QString &playbackStatus)
{
    qDebug() << Q_FUNC_INFO << playbackStatus;
    if (m_PlaybackStatus != playbackStatus) {
        m_PlaybackStatus = playbackStatus;
        Q_EMIT playbackStatusChanged();
    }
}

qlonglong DeclarativeMprisAdaptor::position() const
{
    return m_Position;
}

double DeclarativeMprisAdaptor::rate() const
{
    return m_Rate;
}

void DeclarativeMprisAdaptor::setRate(double value)
{
    if (m_Rate != value) {
        m_Rate = value;
        Q_EMIT rateChanged();
    }
}

bool DeclarativeMprisAdaptor::shuffle() const
{
    return m_Shuffle;
}

void DeclarativeMprisAdaptor::setShuffle(bool value)
{
    if (m_Shuffle != value) {
        m_Shuffle = value;
        Q_EMIT shuffleChanged();
    }
}

double DeclarativeMprisAdaptor::volume() const
{
    return m_Volume;
}

void DeclarativeMprisAdaptor::setVolume(double value)
{
    if (m_Volume != value) {
        m_Volume = value;
        Q_EMIT volumeChanged();
    }
}

void DeclarativeMprisAdaptor::Next()
{
    Q_EMIT nextRequested();
}

void DeclarativeMprisAdaptor::OpenUri(const QString &Uri)
{
    Q_EMIT openUriRequested(Uri);
}

void DeclarativeMprisAdaptor::Pause()
{
    Q_EMIT pauseRequested();
}

void DeclarativeMprisAdaptor::Play()
{
    Q_EMIT playRequested();
}

void DeclarativeMprisAdaptor::PlayPause()
{
    Q_EMIT playPauseRequested();
}

void DeclarativeMprisAdaptor::Previous()
{
    Q_EMIT previousRequested();
}

void DeclarativeMprisAdaptor::Seek(qlonglong Offset)
{
    Q_EMIT seekRequested(Offset);
}

void DeclarativeMprisAdaptor::SetPosition(const QDBusObjectPath &TrackId, qlonglong Position)
{
    Q_EMIT setPositionRequested(TrackId, Position);
}

void DeclarativeMprisAdaptor::Stop()
{
    Q_EMIT stopRequested();
}

void DeclarativeMprisAdaptor::signalPropertyChange(const QString &property, const QVariant &value)
{
    qDebug() << Q_FUNC_INFO << property << value;
    QVariantMap properties;
    properties[property] = value;
    const int ifaceIndex = metaObject()->indexOfClassInfo("D-Bus Interface");
    QDBusMessage msg = QDBusMessage::createSignal(mprisMediaPlayerObjectPath, dBusPropertiesInterface, dBusPropertiesChangedSignal);

    msg << QLatin1String(metaObject()->classInfo(ifaceIndex).value());
    msg << properties;
    msg << QStringList();

    m_dbus.send(msg);
}
