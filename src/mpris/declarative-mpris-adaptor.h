// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <QObject>

#include <QDBusConnection>
#include <QDBusObjectPath>

class DeclarativeMprisAdaptor : public QObject
{
    Q_OBJECT

public:
    explicit DeclarativeMprisAdaptor(bool systemBus = false, QObject *parent = nullptr);
    ~DeclarativeMprisAdaptor() = default;

    Q_PROPERTY(bool CanControl READ canControl NOTIFY canControlChanged)
    bool canControl() const;

    Q_PROPERTY(bool CanGoNext READ canGoNext NOTIFY canGoNextChanged)
    bool canGoNext() const;

    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious NOTIFY canGoPreviousChanged)
    bool canGoPrevious() const;

    Q_PROPERTY(bool CanPause READ canPause NOTIFY canPauseChanged)
    bool canPause() const;

    Q_PROPERTY(bool CanPlay READ canPlay NOTIFY canPlayChanged)
    bool canPlay() const;

    Q_PROPERTY(bool CanSeek READ canSeek NOTIFY canSeekChanged)
    bool canSeek() const;

    Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus NOTIFY loopStatusChanged)
    QString loopStatus() const;
    void setLoopStatus(const QString &value);

    Q_PROPERTY(double MaximumRate READ maximumRate NOTIFY maximumRateChanged)
    double maximumRate() const;

    Q_PROPERTY(QVariantMap Metadata READ metadata NOTIFY metadataChanged)
    QVariantMap metadata() const;

    Q_PROPERTY(double MinimumRate READ minimumRate NOTIFY minimumRateChanged)
    double minimumRate() const;

    Q_PROPERTY(QString PlaybackStatus READ playbackStatus WRITE setPlaybackStatus NOTIFY playbackStatusChanged)
    QString playbackStatus() const;
    void setPlaybackStatus(const QString &playbackStatus);

    Q_PROPERTY(qlonglong Position READ position NOTIFY positionChanged)
    qlonglong position() const;

    Q_PROPERTY(double Rate READ rate WRITE setRate NOTIFY rateChanged)
    double rate() const;
    void setRate(double value);

    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
    bool shuffle() const;
    void setShuffle(bool value);

    Q_PROPERTY(double Volume READ volume WRITE setVolume NOTIFY volumeChanged)
    double volume() const;
    void setVolume(double value);

public Q_SLOTS:
    void Next();
    void OpenUri(const QString &Uri);
    void Pause();
    void Play();
    void PlayPause();
    void Previous();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);
    void Stop();

Q_SIGNALS:
    void Seeked(qlonglong Position);

    void canControlChanged();
    void canGoNextChanged();
    void canGoPreviousChanged();
    void canPauseChanged();
    void canPlayChanged();
    void canSeekChanged();
    void loopStatusChanged();
    void maximumRateChanged();
    void metadataChanged();
    void minimumRateChanged();
    void playbackStatusChanged();
    void positionChanged();
    void rateChanged();
    void shuffleChanged();
    void volumeChanged();

    void nextRequested();
    void openUriRequested(const QString &Uri);
    void pauseRequested();
    void playRequested();
    void playPauseRequested();
    void previousRequested();
    void seekRequested(qlonglong Offset);
    void setPositionRequested(const QDBusObjectPath &TrackId, qlonglong Position);
    void stopRequested();

private:
    void signalPropertyChange(const QString &property, const QVariant &value);

    bool m_CanControl;
    bool m_CanGoNext;
    bool m_CanGoPrevious;
    bool m_CanPause;
    bool m_CanPlay;
    bool m_CanSeek;
    QString m_LoopStatus;
    double m_MaximumRate;
    QVariantMap m_Metadata;
    double m_MinimumRate;
    QString m_PlaybackStatus;
    qlonglong m_Position;
    double m_Rate;
    bool m_Shuffle;
    double m_Volume;

    QDBusConnection m_dbus;
};
