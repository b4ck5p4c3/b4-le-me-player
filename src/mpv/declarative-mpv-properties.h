// SPDX-FileCopyrightText: 2023 George Florea Bănuș <georgefb899@gmail.com>
// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>

class DeclarativeMpvProperties : public QObject
{
    Q_OBJECT

public:
    static DeclarativeMpvProperties *self()
    {
        static DeclarativeMpvProperties p;
        return &p;
    }

    Q_PROPERTY(QString MediaTitle MEMBER MediaTitle CONSTANT)
    const QString MediaTitle{QStringLiteral("media-title")};

    Q_PROPERTY(QString Position MEMBER Position CONSTANT)
    const QString Position{QStringLiteral("time-pos")};

    Q_PROPERTY(QString Duration MEMBER Duration CONSTANT)
    const QString Duration{QStringLiteral("duration")};

    Q_PROPERTY(QString Pause MEMBER Pause CONSTANT)
    const QString Pause{QStringLiteral("pause")};

    Q_PROPERTY(QString Volume MEMBER Volume CONSTANT)
    const QString Volume{QStringLiteral("volume")};

    Q_PROPERTY(QString Mute MEMBER Mute CONSTANT)
    const QString Mute{QStringLiteral("mute")};

    Q_PROPERTY(QString ScriptOpts MEMBER ScriptOpts CONSTANT)
    const QString ScriptOpts{QStringLiteral("script-opts")};

    Q_PROPERTY(QString YtdlFormat MEMBER YtdlFormat CONSTANT)
    const QString YtdlFormat{QStringLiteral("ytdl-format")};

    Q_PROPERTY(QString Video MEMBER Video CONSTANT)
    const QString Video{QStringLiteral("video")};

private:
    explicit DeclarativeMpvProperties(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    DeclarativeMpvProperties(const DeclarativeMpvProperties &) = delete;
    DeclarativeMpvProperties &operator=(const DeclarativeMpvProperties &) = delete;
    DeclarativeMpvProperties(DeclarativeMpvProperties &&) = delete;
    DeclarativeMpvProperties &operator=(DeclarativeMpvProperties &&) = delete;
};
