// SPDX-License-Identifier: BSD-3-Clause
#include "player.h"
#include "volume.h"
#include <QUrl>
#include <cmath>

Player::Player(QObject *parent)
    : QObject(parent)
{
}

Player::~Player()
{
    if (m_handle) {
        mpv_set_wakeup_callback(m_handle, nullptr, nullptr);
        mpv_terminate_destroy(m_handle);
    }
}

QString Player::initialize(const QString &audioOutput)
{
    if (m_handle)
        return "already_initialized";
    m_handle = mpv_create();
    if (!m_handle)
        return "mpv_create_failed";
    for (const auto &option :
         {std::pair{"config", "no"}, {"vid", "no"}, {"idle", "yes"}, {"terminal", "no"}, {"ytdl", "no"}, {"load-scripts", "no"}, {"volume", "0"}}) {
        const int error = mpv_set_option_string(m_handle, option.first, option.second);
        const bool optionalScriptOption = QByteArray(option.first) == "ytdl" || QByteArray(option.first) == "load-scripts";
        if (error < 0 && !(optionalScriptOption && error == MPV_ERROR_OPTION_NOT_FOUND))
            return QString::fromUtf8(mpv_error_string(error));
    }
    if (!audioOutput.isEmpty()) {
        const auto output = audioOutput.toUtf8();
        const int result = mpv_set_option_string(m_handle, "ao", output.constData());
        if (result < 0)
            return QString::fromUtf8(mpv_error_string(result));
    }
    const int error = mpv_initialize(m_handle);
    if (error < 0)
        return QString::fromUtf8(mpv_error_string(error));
    mpv_observe_property(m_handle, 1, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(m_handle, 2, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_handle, 3, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_handle, 4, "duration", MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(m_handle, &Player::wakeup, this);
    m_initialized = true;
    return {};
}

void Player::wakeup(void *context)
{
    auto *self = static_cast<Player *>(context);
    QMetaObject::invokeMethod(
        self,
        [self] {
            self->drainEvents();
        },
        Qt::QueuedConnection);
}

void Player::drainEvents()
{
    while (const auto *event = mpv_wait_event(m_handle, 0)) {
        if (event->event_id == MPV_EVENT_NONE)
            break;
        if (event->event_id == MPV_EVENT_COMMAND_REPLY) {
            const auto it = m_requests.find(event->reply_userdata);
            if (it != m_requests.end()) {
                const auto id = it.value();
                m_requests.erase(it);
                Q_EMIT commandFinished(id, event->error < 0 ? QString::fromUtf8(mpv_error_string(event->error)) : QString());
            }
            continue;
        }
        if (event->event_id == MPV_EVENT_START_FILE)
            m_state.insert("status", "loading");
        else if (event->event_id == MPV_EVENT_FILE_LOADED)
            m_state.insert("status", "loaded");
        else if (event->event_id == MPV_EVENT_PLAYBACK_RESTART)
            m_state.insert("status", "playing");
        else if (event->event_id == MPV_EVENT_END_FILE) {
            const auto *end = static_cast<mpv_event_end_file *>(event->data);
            m_state.insert("status", end->reason == MPV_END_FILE_REASON_ERROR ? "error" : "stopped");
            m_state.insert("error", end->error < 0 ? QString::fromUtf8(mpv_error_string(end->error)) : QString());
        } else if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            const auto *property = static_cast<mpv_event_property *>(event->data);
            if (property->data && property->format == MPV_FORMAT_FLAG)
                m_state.insert(QString::fromUtf8(property->name), bool(*static_cast<int *>(property->data)));
            else if (property->data && property->format == MPV_FORMAT_DOUBLE) {
                const double value = *static_cast<double *>(property->data);
                const auto name = QString::fromUtf8(property->name);
                m_state.insert(name, name == "volume" ? Volume::fromMpv(value) : value);
            }
        } else {
            continue;
        }
        Q_EMIT stateChanged();
    }
}

QString Player::execute(const QJsonObject &command)
{
    if (!m_initialized)
        return "not_initialized";
    const auto requestId = command.value("id").toString();
    if (requestId.size() > 80)
        return "invalid_id";
    if (m_requests.size() >= 5)
        return "busy";
    if (!requestId.isEmpty() && m_requests.values().contains(requestId))
        return "id_in_flight";
    const auto operation = command.value("op").toString();
    QStringList arguments;
    if (operation == "play") {
        const QUrl url(command.value("url").toString(), QUrl::StrictMode);
        if (!url.isValid() || url.host().isEmpty() || !url.userInfo().isEmpty() || (url.scheme() != "http" && url.scheme() != "https"))
            return "invalid_url";
        arguments = {"loadfile", url.toString(), "replace"};
    } else if (operation == "stop") {
        arguments = {"stop"};
    } else if (operation == "pause") {
        if (!command.value("value").isBool())
            return "invalid_pause";
        arguments = {"set", "pause", command.value("value").toBool() ? "yes" : "no"};
    } else if (operation == "volume") {
        const auto value = command.value("value");
        const double number = value.toDouble(-1);
        if (!value.isDouble() || !std::isfinite(number) || number < 0 || number > 100)
            return "invalid_volume";
        arguments = {"set", "volume", QString::number(Volume::toMpv(number), 'g', 17)};
    } else {
        return "unsupported_operation";
    }
    QList<QByteArray> storage;
    for (const auto &argument : arguments)
        storage.append(argument.toUtf8());
    QList<const char *> argv;
    for (const auto &argument : storage)
        argv.append(argument.constData());
    argv.append(nullptr);
    const quint64 sequence = ++m_nextRequest;
    m_requests.insert(sequence, requestId);
    const int error = mpv_command_async(m_handle, sequence, argv.data());
    if (error < 0) {
        m_requests.remove(sequence);
        return QString::fromUtf8(mpv_error_string(error));
    }
    return {};
}

QJsonObject Player::state() const
{
    return m_state;
}
