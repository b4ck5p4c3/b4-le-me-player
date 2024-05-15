// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <QtCore/QMap>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>
#include <QtQml/QtQml>
#ifndef QT_NO_SSL
#include <QSslConfiguration>
#endif

class DeclarativeMqttClient;

class DeclarativeMqttSubscription : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QMqttTopicFilter topic MEMBER m_topic NOTIFY topicChanged)
    QML_UNCREATABLE("Not intended to be creatable")

public:
    DeclarativeMqttSubscription(QMqttSubscription *s, DeclarativeMqttClient *c);
    ~DeclarativeMqttSubscription();

Q_SIGNALS:
    void topicChanged(QString);
    void messageReceived(const QString &msg, const QString &topic);

public slots:
    void handleMessage(const QMqttMessage &qmsg);

private:
    Q_DISABLE_COPY(DeclarativeMqttSubscription)
    QMqttSubscription *sub;
    DeclarativeMqttClient *client;
    QMqttTopicFilter m_topic;
};

class DeclarativeMqttClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hostname READ hostname WRITE setHostname NOTIFY hostnameChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString clientCertPath READ clientCertPath WRITE setClientCertPath NOTIFY clientCertPathChanged)
    Q_PROPERTY(QMqttClient::ClientState state READ state WRITE setState NOTIFY stateChanged)
    QML_NAMED_ELEMENT(MqttClient)
    QML_EXTENDED_NAMESPACE(QMqttClient)
public:
    DeclarativeMqttClient(QObject *parent = nullptr);

    Q_INVOKABLE void connectToHost(bool encrypted = true);
    Q_INVOKABLE void disconnectFromHost();
    Q_INVOKABLE DeclarativeMqttSubscription *subscribe(const QString &topic);

    const QString hostname() const;
    void setHostname(const QString &newHostname);

    int port() const;
    void setPort(int newPort);

    QMqttClient::ClientState state() const;
    void setState(const QMqttClient::ClientState &newState);

    QString username() const;
    void setUsername(const QString &newUserName);

    QString password() const;
    void setPassword(const QString &newPassword);

    QString clientCertPath() const;
    void setClientCertPath(const QString &newClientCertPath);

signals:
    void hostnameChanged();
    void portChanged();
    void stateChanged();
    void usernameChanged();
    void passwordChanged();
    void clientCertPathChanged();

private:
    Q_DISABLE_COPY(DeclarativeMqttClient)
    QMqttClient m_client;
    QString m_username;
    QString m_password;
    QString m_clientCertPath;
#ifndef QT_NO_SSL
    QSslConfiguration m_sslConfiguration;
#endif
};
