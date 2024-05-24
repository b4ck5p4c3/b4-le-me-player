// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: BSD-3-Clause

import LeMePlayer
import QtQuick

Window {
    width: height
    height: uiConstants.windowDefaultHeight
    visible: true
    title: applicationDescription
    color: uiConstants.windowDefaultColor

    MqttClient {
        id: client

        property string leTopic: mqttTopicPrefix
        property string musicPath: "/music"
        property string musicTopic: leTopic + musicPath
        property string voicePath: "/voice"
        property string voiceTopic: leTopic + voicePath
        property string effectsPath: "/effects"
        property string effectsTopic: leTopic + effectsPath
        property var leSubscription: 0

        function parseMessage(payload, topic) {
            let mpvItem = musicMpvItem;
            if (topic.startsWith(musicTopic))
                mpvItem = musicMpvItem;
            else if (topic.startsWith(voiceTopic))
                mpvItem = voiceMpvItem;
            else if (topic.startsWith(effectsTopic))
                mpvItem = effectsMpvItem;
            let command = payload;
            let sourceUrl;
            if (topic.endsWith("url")) {
                sourceUrl = payload;
                command = "url";
            }
            switch (command) {
            case "playPause":
                mpvItem.playPause();
                break;
            case "mute":
                mpvItem.mute = !mpvItem.mute;
                break;
            case "url":
                mpvItem.loadUrl(sourceUrl);
                break;
            default:
                console.log(command, payload, topic);
            }
        }

        function subsctibeToTopics() {
            leSubscription = client.subscribe(leTopic + "/#");
            leSubscription.messageReceived.connect(parseMessage);
        }

        hostname: mqttHostname
        username: mqttUsername
        password: mqttPassword
        port: mqttPort
        clientCertPath: mqttCaFile
        Component.onCompleted: {
            connectToHost(!mqttUnencrypted);
        }
        onStateChanged: {
            if (state === MqttClient.Connected)
                subsctibeToTopics();

        }
    }

    MpvItem {
        id: musicMpvItem

        anchors.fill: parent
        noVideo: true
    }

    Connections {
        property MpvItem mpvItem: musicMpvItem

        function onNextRequested() {
            console.log("TODO: onNextRequested");
        }

        function onOpenUriRequested(uri) {
            mpvItem.loadUrl(uri);
        }

        function onPauseRequested() {
            mpvItem.pause = true;
        }

        function onPlayRequested() {
            mpvItem.pause = false;
        }

        function onPlayPauseRequested() {
            mpvItem.playPause();
        }

        function onPreviousRequested() {
            console.log("TODO: previousRequested");
        }

        function onSeekRequested(offset) {
            mpvItem.position = mpvItem.position - offset;
        }

        function onSetPositionRequested(trackId, position) {
            console.log("TODO: onSetPositionRequested: trackId");
            mpvItem.position = position;
        }

        function onStopRequested() {
            mpvItem.pause = true;
            mpvItem.position = 0;
        }

        target: mprisAdaptor
    }

    MpvItem {
        id: voiceMpvItem

        anchors.fill: parent
        noVideo: true
    }

    MpvItem {
        id: effectsMpvItem

        anchors.fill: parent
        noVideo: true
    }

    UiConstants {
        id: uiConstants
    }

}
