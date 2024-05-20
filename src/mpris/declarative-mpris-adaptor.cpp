// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: BSD-3-Clause

#include "declarative-mpris-adaptor.h"
#include <QDebug>

DeclarativeMprisAdaptor::DeclarativeMprisAdaptor(QObject *parent)
{
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
