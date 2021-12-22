#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QtMqtt>
#include <memory>

#include "config.h"
#include "inputthread.h"

class MqttClientPrivate;
class MqttClient : public QObject
{
    Q_OBJECT
public:
    MqttClient(Config *config, InputHandler *parent = nullptr);
    ~MqttClient() override;

    Q_SLOT void start();
    Q_SLOT void stop();
    Q_SLOT void restart();

    InputHandler *inputHandler() const;
private:
    std::unique_ptr<MqttClientPrivate> d;
};

#endif//MQTTCLIENT_H
