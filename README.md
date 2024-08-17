# b4ck5p4c3 media player

Control (lib)mpv/yt-dlp via MQTT with Qt/QML.

![kirby-headphones.gif](src/qml/kirby-headphones.gif)

- [in russian (на русском)](README.ru.md)

## Features

- encrypted and unencrypted MQTT connection for controlling the player over the network
- system or session D-Bus for interaction with other MPRIS players of the OS (automatically pause a running player if you want to turn on another one)
- 1 main audio stream for music and 2 additional (for example, for sound events or parallel speech output)

## Control commands

See also: [src/qml/Main.qml](src/qml/Main.qml) for MQTT commands parsing logic.

Example commands are provided for utilities of the Mosquitto project.

### Play or pause

Payload message: `playPause`, topic: `bus/devices/le-me/music` (also `../voice` or `../effects`):

```sh
mosquitto_pub -i "human" -h "mqtt.service" -u "username" -P "password" -p 8883 --cafile "/path/to/cert.crt" -t "bus/devices/le-me/music" -m "playPause"
```

### Specify source to play

Payload message: {url стрима}, топик: `bus/devices/le-me/music/url` (also `../voice/url` or `../effects/url`):

- youtube example:

```sh
mosquitto_pub -i "human" -h "mqtt.service" -u "username" -P "password" --cafile "/path/to/cert.crt" -t "bus/devices/le-me/music/url" -m "https://www.youtube.com/watch?v=TBMU_OewAbQ"
```

- hermitage internet radio example:

```sh
mosquitto_pub -i "human" -h "mqtt.service" -u "username" -P "password" --cafile "/path/to/cert.crt" -t "bus/devices/le-me/music/url" -m "https://hermitage.fm/"
```

## Launch player

See also cli manual (`--help`) and [src/main.cpp](src/main.cpp) with cli commands parser.

Example of systemd service to automate player launch after system restart (`/usr/lib/systemd/system/b4-le-me-player.service`):

```ini
[Unit]
Description=b4ck5p4c3 media player example
After=network-online.target
Wants=network-online.target
StartLimitIntervalSec=5
StartLimitBurst=10

[Service]
Type=simple

# So embedded, much hacker
User=root
WorkingDirectory=/path/to/b4-le-me-player/build/src
ExecStart=/path/to/b4-le-me-player/build/src/b4-le-me-player -platform minimal -n "mqtt.service" -u "username" -P "password" -p 8883 -c "/path/to/ca-cert/cert.crt" -S
Restart=always
RestartSec=1
RestartSteps=5
RestartMaxDelaySec=10

[Install]
WantedBy=multi-user.target
```

## Building

### Essential build dependencies (for Ubuntu as an example)

```sh
sudo apt -y install cmake build-essential extra-cmake-modules git
sudo apt-get install libgl1-mesa-dev # QTBUG-108048
sudo apt -y install libmpv-dev
sudo apt -y install python3-pip
sudo apt -y install python3-venv
sudo apt-get install gcc python3-dev
sudo apt -y install vim # handy
```

### python: virtual environment to automate downloading the latest SDK for Qt

```sh
python3 -m venv .pythonvenvforqt
source ./.pythonvenvforqt/bin/activate
```

### aqtinstall: automation of getting pre-built SDK binaries for Qt (version 6.7.0 for example)

See also: https://github.com/miurahr/aqtinstall#install

- getting aqtinstall via pip:

```sh
pip install -U pip
pip install aqtinstall
```

- optionally set up a mirroe due to [Qt Company restrictions on its pre-built binaries](https://forum.qt.io/topic/134724/unlock-qt-in-russia) `.pythonvenvforqt/settings.ini`:

```
[aqt] 
baseurl: baseurl: https://qt-mirror.dannhauer.de/

[requests]
max_retries_on_checksum_error: 1
max_retries_to_retrieve_hash: 1
INSECURE_NOT_FOR_PRODUCTION_ignore_hash: True

[mirrors]
fallbacks:
    https://mirrors.ocf.berkeley.edu/qt
    https://qt.mirror.constant.com/
    https://ftp.acc.umu.se/mirror/qt.io/qtproject/
    https://qtproject.mirror.liquidtelecom.com/
    https://ftp.jaist.ac.jp/pub/qtproject
    http://ftp1.nluug.nl/languages/qt
    https://mirrors.dotsrc.org/qtproject
    https://mirror.yandex.ru/mirrors/qt.io
```

- deploy the required Qt modules (for example, qtbase and qtdeclarative):

```sh
aqt --config .pythonvenvforqt/settings.ini list-qt linux_arm64 desktop
aqt --config .pythonvenvforqt/settings.ini install-qt linux_arm64 desktop 6.7.0 -O qt #--archives icu qtbase qtdeclarative
```

### QtMqtt: building dependencies from sources

```sh
git clone https://github.com/qt/qtmqtt.git
git checkout v6.7.0
mkdir build && cd build
/path/to/qt/6.7.0/gcc_arm64/bin/qt-cmake ..
make
make install
```

(installs to `/path/to/qt/6.7.0/gcc_arm64/...`)

### MpvQt: building dependencies from sources

```sh
git clone https://invent.kde.org/libraries/mpvqt.git
mkdir build && cd build
/path/to/qt/6.7.0/gcc_arm64/bin/qt-cmake -DCMAKE_INSTALL_PREFIX=/path/to/qt/6.7.0/gcc_arm64/ -DCMAKE_BUILD_TYPE=Release ..
```

(installs to `/path/to/qt/6.7.0/gcc_arm64/...`)

### b4-le-me-player: building the project itself

```sh
git clone git clone https://github.com/b4ck5p4c3/b4-le-me-player.git
cd b4-le-me-player
mkdir build && cd build
/path/to/qt/6.7.0/gcc_arm64/bin/qt-cmake -DCMAKE_BUILD_TYPE=Release ..
```


## Architecture

- [src/main.cpp](src/main.cpp) — entry point with launch commands and the declarative QML engine (formally for the GUI, but it is not planned to display it for now)
- [src/qml/Main.qml](src/qml/Main.qml) — entry point with a declarative description of all the player components and connections between them (three mpv players and an mqtt client for them)
- [src/mpv](src/mpv) — MpvQt library wrapper for declarative usage with QML
- [src/mqtt](src/mqtt) — QtMqtt library wrapper for declarative usage with QML
- [src/mpris](src/mpris) — interface for D-Bus, where the pause of other MPRIS players at the system is automated; also an adapter for D-Bus, so that 1 main audio stream for music is also available via MRPIS


