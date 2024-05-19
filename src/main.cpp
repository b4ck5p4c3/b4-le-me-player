// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
//
// SPDX-License-Identifier: BSD-3-Clause

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>

#include "build-constants.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QString applicationName;
    QString applicationVersion;
    QString moduleUri;
    QString applicationDescription;

#ifdef TARGET_NAME
    applicationName = TARGET_NAME;
#endif
#ifdef TARGET_URI
    moduleUri = TARGET_URI;
#endif
#ifdef TARGET_DESCRIPTION
    applicationDescription = TARGET_DESCRIPTION;
#endif
#ifdef TARGET_VERSION
    applicationVersion = TARGET_VERSION;
#endif

    QCoreApplication::setApplicationName(applicationName);
    QCoreApplication::setApplicationVersion(applicationVersion);

    QCommandLineParser parser;
    parser.setApplicationDescription(applicationDescription);
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption mqttHostnameOption(QStringList() << "n"
                                                        << "mqtt-hostname",
                                          QCoreApplication::translate("main", "MQTT client hostname"),
                                          QCoreApplication::translate("main", "hostname"),
                                          QStringLiteral("localhost"));
    parser.addOption(mqttHostnameOption);

    QCommandLineOption mqttUsernameOption(QStringList() << "u"
                                                        << "mqtt-username",
                                          QCoreApplication::translate("main", "MQTT client username"),
                                          QCoreApplication::translate("main", "username"));
    parser.addOption(mqttUsernameOption);

    QCommandLineOption mqttPasswordOption(QStringList() << "P"
                                                        << "mqtt-password",
                                          QCoreApplication::translate("main", "MQTT client password"),
                                          QCoreApplication::translate("main", "password"));
    parser.addOption(mqttPasswordOption);

    QCommandLineOption mqttPortOption(QStringList() << "p"
                                                    << "mqtt-port",
                                      QCoreApplication::translate("main", "MQTT client port"),
                                      QCoreApplication::translate("main", "port"));
    parser.addOption(mqttPortOption);

    QCommandLineOption mqttCaFileOption(QStringList() << "c"
                                                      << "mqtt-cafile",
                                        QCoreApplication::translate("main", "MQTT client path to CA cert file"),
                                        QCoreApplication::translate("main", "path"));
    parser.addOption(mqttCaFileOption);

    QCommandLineOption mqttTopicPrefixOption(QStringList() << "T"
                                                           << "mqtt-topic-prefix",
                                             QCoreApplication::translate("main", "MQTT client prefix for the application topics"),
                                             QCoreApplication::translate("main", "prefix"),
                                             QStringLiteral("bus/devices/le-me"));
    parser.addOption(mqttTopicPrefixOption);

    QCommandLineOption mqttUnencryptedOption(QStringList() << "U"
                                                           << "mqtt-unencrypted",
                                             QCoreApplication::translate("main", "MQTT client initiates an unencrypted connection"));
    parser.addOption(mqttUnencryptedOption);

    parser.process(app);

    bool mqttUnencrypted = parser.isSet(mqttUnencryptedOption);
    QString mqttHostname = parser.value(mqttHostnameOption);
    QString mqttUsername = parser.value(mqttUsernameOption);
    QString mqttPassword = parser.value(mqttPasswordOption);
    quint16 mqttPort;
    mqttPort = static_cast<quint16>(parser.value(mqttPortOption).toUInt());
    if (!parser.isSet(mqttPortOption)) {
        if (mqttUnencrypted) {
            mqttPort = 1883;
        } else {
            mqttPort = 8883;
        }
    }
    QString mqttCaFile = parser.value(mqttCaFileOption);
    QString mqttTopicPrefix = parser.value(mqttTopicPrefixOption);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty(QStringLiteral("applicationDescription"), QVariant::fromValue(applicationDescription));
    engine.rootContext()->setContextProperty(QStringLiteral("mqttHostname"), QVariant::fromValue(mqttHostname));
    engine.rootContext()->setContextProperty(QStringLiteral("mqttUsername"), QVariant::fromValue(mqttUsername));
    engine.rootContext()->setContextProperty(QStringLiteral("mqttPassword"), QVariant::fromValue(mqttPassword));
    engine.rootContext()->setContextProperty(QStringLiteral("mqttPort"), QVariant::fromValue(mqttPort));
    engine.rootContext()->setContextProperty(QStringLiteral("mqttCaFile"), QVariant::fromValue(mqttCaFile));
    engine.rootContext()->setContextProperty(QStringLiteral("mqttTopicPrefix"), QVariant::fromValue(mqttTopicPrefix));
    engine.rootContext()->setContextProperty(QStringLiteral("mqttUnencrypted"), QVariant::fromValue(mqttUnencrypted));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.loadFromModule(moduleUri, "Main");

    return app.exec();
}
