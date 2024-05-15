// SPDX-FileCopyrightText: 2023 George Florea Bănuș <georgefb899@gmail.com>
// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
//
// SPDX-License-Identifier: MIT

#include "declarative-mpv-item.h"

#include <MpvController>

#include "declarative-mpv-properties.h"

MpvItem::MpvItem(QQuickItem *parent)
    : MpvAbstractItem(parent)
{
    observeProperty(DeclarativeMpvProperties::self()->MediaTitle, MPV_FORMAT_STRING);
    observeProperty(DeclarativeMpvProperties::self()->Position, MPV_FORMAT_DOUBLE);
    observeProperty(DeclarativeMpvProperties::self()->Duration, MPV_FORMAT_DOUBLE);
    observeProperty(DeclarativeMpvProperties::self()->Pause, MPV_FORMAT_FLAG);
    observeProperty(DeclarativeMpvProperties::self()->Volume, MPV_FORMAT_INT64);

    setupConnections();

    // since this is async the effects are not immediately visible
    // to do something after the property was set do it in onAsyncReply
    // use the id to identify the correct call
    setPropertyAsync(QStringLiteral("volume"), 99, static_cast<int>(MpvItem::AsyncIds::SetVolume));
    // setProperty(QStringLiteral("mute"), true);

    Q_EMIT setProperty(DeclarativeMpvProperties::self()->ScriptOpts, QStringLiteral("ytdl_hook-ytdl_path=%1").arg(QStringLiteral("yt-dlp")));
    Q_EMIT setProperty(DeclarativeMpvProperties::self()->YtdlFormat, QStringLiteral("bestvideo[height<=720]+bestaudio/best"));

    // since this is async the effects are not immediately visible
    // to get the value do it in onGetPropertyReply
    // use the id to identify the correct call
    getPropertyAsync(DeclarativeMpvProperties::self()->Volume, static_cast<int>(MpvItem::AsyncIds::GetVolume));
}

void MpvItem::setupConnections()
{
    // clang-format off
    connect(mpvController(), &MpvController::propertyChanged,
            this, &MpvItem::onPropertyChanged, Qt::QueuedConnection);

    connect(mpvController(), &MpvController::fileStarted,
            this, &MpvItem::fileStarted, Qt::QueuedConnection);

    connect(mpvController(), &MpvController::fileLoaded,
            this, &MpvItem::fileLoaded, Qt::QueuedConnection);

    connect(mpvController(), &MpvController::endFile,
            this, &MpvItem::endFile, Qt::QueuedConnection);

    connect(mpvController(), &MpvController::videoReconfig,
            this, &MpvItem::videoReconfig, Qt::QueuedConnection);

    connect(mpvController(), &MpvController::asyncReply,
            this, &MpvItem::onAsyncReply, Qt::QueuedConnection);
    // clang-format on
}

void MpvItem::onPropertyChanged(const QString &property, const QVariant &value)
{
    if (property == DeclarativeMpvProperties::self()->MediaTitle) {
        Q_EMIT mediaTitleChanged();

    } else if (property == DeclarativeMpvProperties::self()->Position) {
        m_formattedPosition = formatTime(value.toDouble());
        Q_EMIT positionChanged();

    } else if (property == DeclarativeMpvProperties::self()->Duration) {
        m_formattedDuration = formatTime(value.toDouble());
        Q_EMIT durationChanged();

    } else if (property == DeclarativeMpvProperties::self()->Pause) {
        Q_EMIT pauseChanged();

    } else if (property == DeclarativeMpvProperties::self()->Volume) {
        Q_EMIT volumeChanged();
    }
}

void MpvItem::onAsyncReply(const QVariant &data, mpv_event event)
{
    switch (static_cast<AsyncIds>(event.reply_userdata)) {
    case AsyncIds::None: {
        break;
    }
    case AsyncIds::SetVolume: {
        qDebug() << "onSetPropertyReply" << event.reply_userdata;
        break;
    }
    case AsyncIds::GetVolume: {
        qDebug() << "onGetPropertyReply" << event.reply_userdata << data;
        break;
    }
    case AsyncIds::ExpandText: {
        qDebug() << "onGetPropertyReply" << event.reply_userdata << data;
        break;
    }
    }
}

QString MpvItem::formatTime(const double time)
{
    int totalNumberOfSeconds = static_cast<int>(time);
    int seconds = totalNumberOfSeconds % 60;
    int minutes = (totalNumberOfSeconds / 60) % 60;
    int hours = (totalNumberOfSeconds / 60 / 60);

    QString timeString =
        QStringLiteral("%1:%2:%3").arg(hours, 2, 10, QLatin1Char('0')).arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));

    return timeString;
}

void MpvItem::loadFile(const QString &file)
{
    auto url = QUrl::fromUserInput(file);
    if (m_currentUrl != url) {
        m_currentUrl = url;
        Q_EMIT currentUrlChanged();
    }

    Q_EMIT command(QStringList() << QStringLiteral("loadfile") << m_currentUrl.toLocalFile());
    setPause(false);
}

void MpvItem::loadUrl(const QUrl &url)
{
    if (m_currentUrl != url) {
        m_currentUrl = url;
        Q_EMIT currentUrlChanged();
    }
    Q_EMIT command(QStringList() << QStringLiteral("loadfile") << m_currentUrl.toString());
    setPause(false);
}

void MpvItem::playPause()
{
    setPause(!pause());
}

QString MpvItem::mediaTitle()
{
    return getProperty(DeclarativeMpvProperties::self()->MediaTitle).toString();
}

double MpvItem::position()
{
    return getProperty(DeclarativeMpvProperties::self()->Position).toDouble();
}

void MpvItem::setPosition(double value)
{
    if (qFuzzyCompare(value, position())) {
        return;
    }
    Q_EMIT setPropertyAsync(DeclarativeMpvProperties::self()->Position, value);
}

double MpvItem::duration()
{
    return getProperty(DeclarativeMpvProperties::self()->Duration).toDouble();
}

bool MpvItem::pause()
{
    return getProperty(DeclarativeMpvProperties::self()->Pause).toBool();
}

void MpvItem::setPause(bool value)
{
    if (value == pause()) {
        return;
    }
    Q_EMIT setPropertyAsync(DeclarativeMpvProperties::self()->Pause, value);
}

int MpvItem::volume()
{
    return getProperty(DeclarativeMpvProperties::self()->Volume).toInt();
}

void MpvItem::setVolume(int value)
{
    if (value == volume()) {
        return;
    }
    Q_EMIT setPropertyAsync(DeclarativeMpvProperties::self()->Volume, value);
}

QString MpvItem::formattedDuration() const
{
    return m_formattedDuration;
}

QString MpvItem::formattedPosition() const
{
    return m_formattedPosition;
}

QUrl MpvItem::currentUrl() const
{
    return m_currentUrl;
}

bool MpvItem::noVideo() const
{
    return m_noVideo;
}

void MpvItem::setNoVideo(bool noVideo)
{
    if (m_noVideo == noVideo)
        return;
    m_noVideo = noVideo;
    QString videoValue = m_noVideo ? QStringLiteral("no") : QStringLiteral("auto");
    Q_EMIT setProperty(DeclarativeMpvProperties::self()->Video, videoValue);
    emit noVideoChanged();
}
