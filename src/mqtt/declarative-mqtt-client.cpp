// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: BSD-3-Clause

#include "declarative-mqtt-client.h"
#include <QDebug>

DeclarativeMqttSubscription::DeclarativeMqttSubscription(QMqttSubscription *s, DeclarativeMqttClient *c)
    : sub(s)
    , client(c)
{
    connect(sub, &QMqttSubscription::messageReceived, this, &DeclarativeMqttSubscription::handleMessage);
    m_topic = sub->topic();
}

DeclarativeMqttSubscription::~DeclarativeMqttSubscription()
{
}

DeclarativeMqttClient::DeclarativeMqttClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_client, &QMqttClient::hostnameChanged, this, &DeclarativeMqttClient::hostnameChanged);
    connect(&m_client, &QMqttClient::portChanged, this, &DeclarativeMqttClient::portChanged);
    connect(&m_client, &QMqttClient::stateChanged, this, &DeclarativeMqttClient::stateChanged);
}

void DeclarativeMqttClient::connectToHost(bool encrypted)
{
    if (encrypted) {
        m_client.connectToHost();
    } else {
        m_client.connectToHostEncrypted(m_sslConfiguration);
    }
}

void DeclarativeMqttClient::disconnectFromHost()
{
    m_client.disconnectFromHost();
}

DeclarativeMqttSubscription *DeclarativeMqttClient::subscribe(const QString &topic)
{
    auto sub = m_client.subscribe(topic, 0);
    auto result = new DeclarativeMqttSubscription(sub, this);
    return result;
}

void DeclarativeMqttSubscription::handleMessage(const QMqttMessage &qmsg)
{
    Q_EMIT messageReceived(qmsg.payload(), qmsg.topic().name());
}

const QString DeclarativeMqttClient::hostname() const
{
    return m_client.hostname();
}

void DeclarativeMqttClient::setHostname(const QString &newHostname)
{
    m_client.setHostname(newHostname);
}

int DeclarativeMqttClient::port() const
{
    return m_client.port();
}

void DeclarativeMqttClient::setPort(int newPort)
{
    if (newPort < 0 || newPort > std::numeric_limits<quint16>::max()) {
        qWarning() << "Trying to set invalid port number";
        return;
    }
    m_client.setPort(static_cast<quint16>(newPort));
}

QMqttClient::ClientState DeclarativeMqttClient::state() const
{
    return m_client.state();
}

void DeclarativeMqttClient::setState(const QMqttClient::ClientState &newState)
{
    m_client.setState(newState);
}

QString DeclarativeMqttClient::username() const
{
    return m_username;
}

void DeclarativeMqttClient::setUsername(const QString &newUserName)
{
    if (m_username == newUserName)
        return;
    m_username = newUserName;
    m_client.setUsername(m_username);
    emit usernameChanged();
}

QString DeclarativeMqttClient::password() const
{
    return m_password;
}

void DeclarativeMqttClient::setPassword(const QString &newPassword)
{
    if (m_password == newPassword)
        return;
    m_password = newPassword;
    m_client.setPassword(m_password);
    emit passwordChanged();
}

QString DeclarativeMqttClient::clientCertPath() const
{
    return m_clientCertPath;
}

void DeclarativeMqttClient::setClientCertPath(const QString &newClientCertPath)
{
    if (m_clientCertPath == newClientCertPath)
        return;
    m_clientCertPath = newClientCertPath;
#ifdef QT_NO_SSL
    qWarning() << "Qt has not been compiled with SSL support.";
    return;
#endif
    QList<QSslCertificate> certs;
    QList<QString> fileNames;
    fileNames.append(m_clientCertPath);
    for (const auto &it : fileNames) {
        auto certificates = QSslCertificate::fromPath(it);
        if (certificates.isEmpty())
            qWarning() << "File " << it << " does not contain any certificates";
        certs.append(certificates);
    }
    if (certs.isEmpty()) {
        qWarning() << "No certificate could be loaded.";
        return;
    }
    m_sslConfiguration.setCaCertificates(certs);
    emit clientCertPathChanged();
}
