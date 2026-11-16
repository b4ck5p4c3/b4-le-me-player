// SPDX-License-Identifier: BSD-3-Clause
#include "player.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTimer>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>
#include <clocale>
#include <memory>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    std::setlocale(LC_NUMERIC, "C");
    QCommandLineParser options;
    options.addHelpOption();
    options.addOption({"host", "MQTT broker", "host", "localhost"});
    options.addOption({"port", "MQTT port", "port", "1883"});
    options.addOption({"topic", "Exclusive MQTT namespace", "topic", "leme/player"});
    options.addOption({"composition", "Trusted local QML composition", "path"});
    options.addOption({"audio-output", "mpv audio output (null for silent tests)", "output"});
    options.process(app);
    bool valid = false;
    const auto port = options.value("port").toUInt(&valid);
    if (!valid || port == 0 || port > 65535)
        qFatal("Invalid port");
    const auto topic = options.value("topic");
    if (topic.isEmpty() || topic.contains('+') || topic.contains('#'))
        qFatal("Invalid topic");
    Player player;
    const auto error = player.initialize(options.value("audio-output"));
    if (!error.isEmpty())
        qFatal("Player initialization: %s", qPrintable(error));
    QQmlEngine engine;
    QQmlEngine::setObjectOwnership(&player, QQmlEngine::CppOwnership);
    engine.rootContext()->setContextProperty("player", &player);
    QQmlComponent composition(&engine);
    std::unique_ptr<QObject> root;
    if (options.isSet("composition")) {
        composition.loadUrl(QUrl::fromLocalFile(options.value("composition")));
        root.reset(composition.create());
        if (!root)
            qFatal("QML composition: %s", qPrintable(composition.errorString()));
    }
    QMqttClient mqtt;
    mqtt.setHostname(options.value("host"));
    mqtt.setPort(quint16(port));
    mqtt.setUsername(qEnvironmentVariable("LEME_MQTT_USERNAME"));
    mqtt.setPassword(qEnvironmentVariable("LEME_MQTT_PASSWORD"));
    auto publishState = [&] {
        mqtt.publish(topic + "/state", QJsonDocument(player.state()).toJson(QJsonDocument::Compact), 0, true);
    };
    QObject::connect(&player, &Player::stateChanged, &app, publishState);
    QObject::connect(&player, &Player::commandFinished, &app, [&](const QString &id, const QString &error) {
        mqtt.publish(topic + "/result",
                     QJsonDocument(QJsonObject{{"id", id}, {"phase", "executed"}, {"ok", error.isEmpty()}, {"error", error}}).toJson(QJsonDocument::Compact));
    });
    QObject::connect(&mqtt, &QMqttClient::connected, &app, [&] {
        publishState();
        auto *subscription = mqtt.subscribe(topic + "/command");
        if (!subscription)
            return;
        QObject::connect(subscription, &QMqttSubscription::messageReceived, &app, [&](const QMqttMessage &message) {
            if (message.retain() || message.payload().size() > 4096)
                return;
            QJsonParseError parseError;
            const auto doc = QJsonDocument::fromJson(message.payload(), &parseError);
            const QString result = parseError.error == QJsonParseError::NoError && doc.isObject() ? player.execute(doc.object()) : "invalid_json";
            mqtt.publish(topic + "/result",
                         QJsonDocument(
                             QJsonObject{{"id", doc.object().value("id").toString()}, {"phase", "accepted"}, {"accepted", result.isEmpty()}, {"error", result}})
                             .toJson(QJsonDocument::Compact));
        });
    });
    QTimer reconnect;
    reconnect.setInterval(3000);
    QObject::connect(&reconnect, &QTimer::timeout, &app, [&] {
        if (mqtt.state() == QMqttClient::Disconnected)
            mqtt.connectToHost();
    });
    reconnect.start();
    mqtt.connectToHost();
    return app.exec();
}
