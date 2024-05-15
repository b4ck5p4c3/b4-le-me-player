﻿// SPDX-FileCopyrightText: 2023 George Florea Bănuș <georgefb899@gmail.com>
// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <MpvAbstractItem>

class MpvRenderer;

class MpvItem : public MpvAbstractItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MpvItem)

public:
    explicit MpvItem(QQuickItem *parent = nullptr);
    ~MpvItem() = default;

    enum class AsyncIds {
        None,
        SetVolume,
        GetVolume,
        ExpandText,
    };
    Q_ENUM(AsyncIds)

    Q_PROPERTY(QString mediaTitle READ mediaTitle NOTIFY mediaTitleChanged)
    QString mediaTitle();

    Q_PROPERTY(double position READ position WRITE setPosition NOTIFY positionChanged)
    double position();
    void setPosition(double value);

    Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
    double duration();

    Q_PROPERTY(QString formattedPosition READ formattedPosition NOTIFY positionChanged)
    QString formattedPosition() const;

    Q_PROPERTY(QString formattedDuration READ formattedDuration NOTIFY durationChanged)
    QString formattedDuration() const;

    Q_PROPERTY(bool pause READ pause WRITE setPause NOTIFY pauseChanged)
    bool pause();
    void setPause(bool value);

    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    int volume();
    void setVolume(int value);

    Q_PROPERTY(QUrl currentUrl READ currentUrl NOTIFY currentUrlChanged)
    QUrl currentUrl() const;

    Q_PROPERTY(bool noVideo READ noVideo WRITE setNoVideo NOTIFY noVideoChanged)
    bool noVideo() const;
    void setNoVideo(bool noVideo);

    Q_INVOKABLE void loadFile(const QString &file);
    Q_INVOKABLE void loadUrl(const QUrl &url);
    Q_INVOKABLE void playPause();

Q_SIGNALS:
    void mediaTitleChanged();
    void currentUrlChanged();
    void positionChanged();
    void durationChanged();
    void pauseChanged();
    void volumeChanged();
    void noVideoChanged();

    void fileStarted();
    void fileLoaded();
    void endFile(QString reason);
    void videoReconfig();

private:
    void setupConnections();
    void onPropertyChanged(const QString &property, const QVariant &value);
    void onAsyncReply(const QVariant &data, mpv_event event);
    QString formatTime(const double time);

    double m_position{0.0};
    QString m_formattedPosition;
    double m_duration{0.0};
    QString m_formattedDuration;
    QUrl m_currentUrl;
    bool m_noVideo;
};
