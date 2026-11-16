// SPDX-License-Identifier: BSD-3-Clause
#include "headless/player.h"
#include "headless/volume.h"
#include <QDataStream>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>
#include <clocale>
#include <memory>

class PlayerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void volumeScale()
    {
        QCOMPARE(Volume::toMpv(0), 0.0);
        QCOMPARE(Volume::toMpv(100), 100.0);
        QVERIFY(std::abs(Volume::toMpv(1) - 10.0) < 1e-10);
        double previous = 0;
        for (int value = 1; value <= 100; ++value) {
            const double backend = Volume::toMpv(value);
            QVERIFY(backend > previous);
            QVERIFY(std::abs(Volume::fromMpv(backend) - value) < 1e-9);
            previous = backend;
        }
    }
    void validation()
    {
        Player player;
        QCOMPARE(player.execute({{"op", "stop"}}), QString("not_initialized"));
        QVERIFY(player.initialize("null").isEmpty());
        QCOMPARE(player.initialize("null"), QString("already_initialized"));
        QCOMPARE(player.execute({{"op", "play"}, {"url", "file:///tmp/audio.wav"}}), QString("invalid_url"));
        QCOMPARE(player.execute({{"op", "pause"}, {"value", "yes"}}), QString("invalid_pause"));
        QCOMPARE(player.execute({{"op", "volume"}, {"value", 101}}), QString("invalid_volume"));
        QVERIFY(player.execute({{"op", "volume"}, {"value", 0}}).isEmpty());
        QTRY_COMPARE(player.state().value("volume").toDouble(-1), 0.0);
    }

    void asyncResults()
    {
        Player player;
        QVERIFY(player.initialize("null").isEmpty());
        QSignalSpy finished(&player, &Player::commandFinished);
        QVERIFY(player.execute({{"id", "volume-1"}, {"op", "volume"}, {"value", 12}}).isEmpty());
        QCOMPARE(player.execute({{"id", "volume-1"}, {"op", "volume"}, {"value", 20}}), QString("id_in_flight"));
        QTRY_COMPARE(finished.size(), 1);
        QCOMPARE(finished.at(0).at(0).toString(), QString("volume-1"));
        QVERIFY(finished.at(0).at(1).toString().isEmpty());
        QTRY_VERIFY(std::abs(player.state().value("volume").toDouble(-1) - 12.0) < 1e-4);
        for (int i = 0; i < 5; ++i)
            QVERIFY(player.execute({{"op", "volume"}, {"value", i}}).isEmpty());
        QCOMPARE(player.execute({{"op", "volume"}, {"value", 0}}), QString("busy"));
        QTRY_COMPARE(finished.size(), 6);
        QVERIFY(player.execute({{"op", "volume"}, {"value", 0}}).isEmpty());
    }

    void httpPlayback()
    {
        QByteArray wav;
        QDataStream stream(&wav, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        const quint32 samples = 16000 * 5;
        stream.writeRawData("RIFF", 4);
        stream << quint32(36 + samples * 2);
        stream.writeRawData("WAVEfmt ", 8);
        stream << quint32(16) << quint16(1) << quint16(1) << quint32(16000) << quint32(32000) << quint16(2) << quint16(16);
        stream.writeRawData("data", 4);
        stream << quint32(samples * 2);
        wav.append(QByteArray(samples * 2, 0));
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost));
        connect(&server, &QTcpServer::newConnection, &server, [&] {
            while (auto *socket = server.nextPendingConnection()) {
                auto request = std::make_shared<QByteArray>();
                connect(socket, &QTcpSocket::readyRead, socket, [socket, request, wav] {
                    request->append(socket->readAll());
                    if (!request->contains("\r\n\r\n"))
                        return;
                    socket->disconnect(socket, &QTcpSocket::readyRead, nullptr, nullptr);
                    if (request->startsWith("GET /pending"))
                        return;
                    if (request->startsWith("GET /missing")) {
                        socket->write("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
                        socket->disconnectFromHost();
                        return;
                    }
                    socket->write("HTTP/1.1 200 OK\r\nContent-Type: audio/wav\r\nContent-Length: " + QByteArray::number(wav.size())
                                  + "\r\nConnection: close\r\n\r\n" + wav);
                    socket->disconnectFromHost();
                });
                connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
            }
        });
        Player player;
        QVERIFY(player.initialize("null").isEmpty());
        const auto url = QString("http://127.0.0.1:%1/test.wav").arg(server.serverPort());
        QVERIFY(player.execute({{"op", "play"}, {"url", url}}).isEmpty());
        QTRY_COMPARE_WITH_TIMEOUT(player.state().value("status").toString(), QString("playing"), 5000);
        const auto firstId = player.state().value("playbackId").toString();
        QVERIFY(!firstId.isEmpty());
        QCOMPARE(player.execute({{"op", "stop"}, {"playbackId", "wrong"}}), QString("stale_playback"));
        QVERIFY(player.execute({{"op", "pause"}, {"playbackId", firstId}, {"value", true}}).isEmpty());
        QTRY_VERIFY(player.state().value("pause").toBool());
        QVERIFY(player.execute({{"op", "stop"}}).isEmpty());
        QTRY_COMPARE(player.state().value("status").toString(), QString("stopped"));
        const auto missing = QString("http://127.0.0.1:%1/missing").arg(server.serverPort());
        QVERIFY(player.execute({{"op", "play"}, {"url", missing}}).isEmpty());
        QTRY_COMPARE_WITH_TIMEOUT(player.state().value("status").toString(), QString("error"), 5000);
        QVERIFY(!player.state().value("error").toString().isEmpty());
        QVERIFY(player.execute({{"op", "pause"}, {"value", false}}).isEmpty());
        QTRY_VERIFY(!player.state().value("pause").toBool());
        QVERIFY(player.execute({{"op", "play"}, {"url", url}, {"playbackId", "second"}}).isEmpty());
        QTRY_COMPARE_WITH_TIMEOUT(player.state().value("status").toString(), QString("playing"), 5000);
        QCOMPARE(player.state().value("playbackId").toString(), QString("second"));
        QCOMPARE(player.execute({{"op", "stop"}, {"playbackId", firstId}}), QString("stale_playback"));
        QCOMPARE(player.execute({{"op", "play"}, {"url", url}, {"playbackId", "second"}}), QString("playback_id_reused"));
        QVERIFY(player.state().value("error").toString().isEmpty());
        QVERIFY(player.execute({{"op", "play"}, {"url", url}, {"playbackId", "replacement"}}).isEmpty());
        QCOMPARE(player.execute({{"op", "stop"}, {"playbackId", "second"}}), QString("stale_playback"));
        QTRY_COMPARE_WITH_TIMEOUT(player.state().value("playbackId").toString(), QString("replacement"), 5000);
        QTRY_COMPARE_WITH_TIMEOUT(player.state().value("status").toString(), QString("playing"), 5000);
        QTRY_COMPARE_WITH_TIMEOUT(player.state().value("status").toString(), QString("finished"), 8000);
        QCOMPARE(player.state().value("playbackId").toString(), QString("replacement"));
        const auto pending = QString("http://127.0.0.1:%1/pending").arg(server.serverPort());
        QVERIFY(player.execute({{"op", "play"}, {"url", pending}, {"playbackId", "pending"}}).isEmpty());
        QVERIFY(player.execute({{"op", "stop"}, {"playbackId", "pending"}}).isEmpty());
        QTRY_COMPARE_WITH_TIMEOUT(player.state().value("playbackId").toString(), QString("pending"), 5000);
        QTRY_COMPARE_WITH_TIMEOUT(player.state().value("status").toString(), QString("stopped"), 5000);
    }

    void qmlWithoutQuick()
    {
        Player player;
        QVERIFY(player.initialize("null").isEmpty());
        QQmlEngine engine;
        QQmlEngine::setObjectOwnership(&player, QQmlEngine::CppOwnership);
        engine.rootContext()->setContextProperty("player", &player);
        QQmlComponent component(&engine);
        component.setData("import QtQml\nQtObject { readonly property string status: player.playbackState.status }", QUrl());
        std::unique_ptr<QObject> root(component.create());
        QVERIFY2(root != nullptr, qPrintable(component.errorString()));
        QCOMPARE(root->property("status").toString(), QString("idle"));
    }
};
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    std::setlocale(LC_NUMERIC, "C");
    PlayerTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "test_player.moc"
