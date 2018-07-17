#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "AudioInput.h"


int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    AudioInput* audioInput = new AudioInput();
    audioInput->startCapture();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("audioInput", audioInput);
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
