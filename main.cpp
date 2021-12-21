#include <QCoreApplication>

#include "inputthread.h"
#include "mqttclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    InputHandler inputHandler;
    MqttClient mqttClient(&inputHandler);
    mqttClient.start();
    app.exec();
}
