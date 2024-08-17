# медиаплеер хакспейса b4ck5p4c3

Написанный на Qt/QML плеер на основе (lib)mpv/yt-dlp с управлением через MQTT.

![kirby-headphones.gif](src/qml/kirby-headphones.gif)

- [in english (на английском)](README.md)

## Возможности

- шифрованное или нешифрованное MQTT соединение для управления плеером по сети
- системная или сессионная dbus-шина для взаимодействия с остальными MPRIS-плеерами в ОС (автоматически ставить на паузу запущенный плеер, если хочется включить другой)
- 1 основной аудио-поток для музыки и 2 дополнительных (например, для озвучивания событий или параллельного вывода речи)

## Управление

См. также [src/qml/Main.qml](src/qml/Main.qml) с логикой обработки MQTT-команд.

Примеры команд указаны для утилит проекта Mosquitto.

### Пауза-плей

Сообщение: `playPause`, топик: `bus/devices/le-me/music` (или `../voice`, или `../effects`):

```sh
mosquitto_pub -i "human" -h "mqtt.service" -u "username" -P "password" -p 8883 --cafile "/path/to/cert.crt" -t "bus/devices/le-me/music" -m "playPause"
```

### Указать источник для воспроизведения

Сообщение: {url стрима}, топик: `bus/devices/le-me/music/url` (или `../voice/url`, или `../effects/url`):

- на примере youtube:

```sh
mosquitto_pub -i "human" -h "mqtt.service" -u "username" -P "password" --cafile "/path/to/cert.crt" -t "bus/devices/le-me/music/url" -m "https://www.youtube.com/watch?v=TBMU_OewAbQ"
```

- на примере радио hermitage:

```sh
mosquitto_pub -i "human" -h "mqtt.service" -u "username" -P "password" --cafile "/path/to/cert.crt" -t "bus/devices/le-me/music/url" -m "https://hermitage.fm/"
```

## Запуск плеера

См. также справку (`--help`) и [src/main.cpp](src/main.cpp), где описаны команды запуска.

Пример systemd-службы для автозагрузки плеера с системой (`/usr/lib/systemd/system/b4-le-me-player.service`):

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

## Сборка

### Базовые зависимости сборки (на примере Ubuntu)

```sh
sudo apt -y install cmake build-essential extra-cmake-modules git
sudo apt-get install libgl1-mesa-dev # QTBUG-108048
sudo apt -y install libmpv-dev
sudo apt -y install python3-pip
sudo apt -y install python3-venv
sudo apt-get install gcc python3-dev
sudo apt -y install vim # handy
```

### python: виртуальная среда для автоматизации скачивания актуального SDK для Qt

```sh
python3 -m venv .pythonvenvforqt
source ./.pythonvenvforqt/bin/activate
```

### aqtinstall: автоматизация получения готовых бинарников SDK для Qt (на примере 6.7.0)

См. также: https://github.com/miurahr/aqtinstall#install

- получаем aqtinstall из pip:

```sh
pip install -U pip
pip install aqtinstall
```

- по необходимости указываем зеркало в силу [ограничений Qt Company на свои готовые бинарники](https://forum.qt.io/topic/134724/unlock-qt-in-russia) `.pythonvenvforqt/settings.ini`:

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

- разворачиваем требуемые Qt модули (например, qtbase и qtdeclarative):

```sh
aqt --config .pythonvenvforqt/settings.ini list-qt linux_arm64 desktop
aqt --config .pythonvenvforqt/settings.ini install-qt linux_arm64 desktop 6.7.0 -O qt #--archives icu qtbase qtdeclarative
```

### QtMqtt: сборка зависимости из исходников

```sh
git clone https://github.com/qt/qtmqtt.git
git checkout v6.7.0
mkdir build && cd build
/path/to/qt/6.7.0/gcc_arm64/bin/qt-cmake ..
make
make install
```

(установка в `/path/to/qt/6.7.0/gcc_arm64/...`)

### MpvQt: сборка зависимости из исходников

```sh
git clone https://invent.kde.org/libraries/mpvqt.git
mkdir build && cd build
/path/to/qt/6.7.0/gcc_arm64/bin/qt-cmake -DCMAKE_INSTALL_PREFIX=/path/to/qt/6.7.0/gcc_arm64/ -DCMAKE_BUILD_TYPE=Release ..
```

(установка в `/path/to/qt/6.7.0/gcc_arm64/...`)

### b4-le-me-player: сборка самого проекта

```sh
git clone git clone https://github.com/b4ck5p4c3/b4-le-me-player.git
cd b4-le-me-player
mkdir build && cd build
/path/to/qt/6.7.0/gcc_arm64/bin/qt-cmake -DCMAKE_BUILD_TYPE=Release ..
```


## Архитектура

- [src/main.cpp](src/main.cpp) — точка входа, где обрабатываются команды запуска и загружается декларативный QML-движок (формально для GUI, но выводить его сейчас не задумано)
- [src/qml/Main.qml](src/qml/Main.qml) — точка входа с декларативным описанием всех компонентов плеера и связей между ними (три mpv плеера и mqtt-клиент для них)
- [src/mpv](src/mpv) — обертка над MpvQt для декларативной работы из QML
- [src/mqtt](src/mqtt) — обертка над QtMqtt для декларативной работы из QML
- [src/mpris](src/mpris) — интерфейс для D-Bus, где автоматизирована пауза остальных mpris-плееров в системе, а также адаптор для D-Bus, чтобы 1 основной аудио-поток для музыки тоже был доступен как mpris-плеер



