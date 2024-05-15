// SPDX-FileCopyrightText: 2024 Alexey Andreyev <aa13q@ya.ru>
//
// SPDX-License-Identifier: BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>

#include "build-constants.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    QString moduleUri;

#ifdef TARGET_URI
    moduleUri = TARGET_URI;
#endif

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
