// SPDX-License-Identifier: BSD-3-Clause
import QtQml

QtObject {
    readonly property var playback: player.playbackState
    readonly property bool paused: playback.pause === true
    readonly property string status: playback.status || "idle"
}
