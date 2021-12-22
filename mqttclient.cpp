#include "mqttclient.h"

#include <QDebug>


class Subscription {
public:
    Subscription(MqttClient *q, Config *config, QMqttSubscription *subscription)
        : q(q)
        , config(config)
        , subscription(subscription)
    {
        QObject::connect(subscription, &QMqttSubscription::messageReceived, q, [this,config,subscription,q](const QMqttMessage &msg){
            qDebug() << "Received message" << msg.payload() << "for topic" << subscription->topic().filter();
            q->inputHandler()->handleKeyPressed(config->charForTopic(subscription->topic().filter()));
        });
        QObject::connect(subscription, &QMqttSubscription::stateChanged, q, [this](){});
        QObject::connect(subscription, &QMqttSubscription::qosChanged, q, [this](){});
        qDebug() << "Subscribed to" << subscription->topic().filter() << "with the parent" << q;
    }
    MqttClient* q;
    Config *config;
    QMqttSubscription *subscription;
};

class MqttClientPrivate {
public:
    MqttClientPrivate(MqttClient *q)
        : q(q)
    {}
    MqttClient *q;
    Config *config{nullptr};
    InputHandler *inputHandler{nullptr};;

    QMqttClient *client{nullptr};
    QList<Subscription> subscriptions;

    void handleSubscription(QMqttSubscription *sub) {
        if (sub) {
            subscriptions << Subscription(q, config, sub);
        } else {
            qWarning() << "Could not subscribe! Is the connection valid?";
        }
    }
};

MqttClient::MqttClient(Config *config, InputHandler *parent)
    : QObject(parent)
    , d(new MqttClientPrivate(this))
{
    d->config = config;
    d->inputHandler = parent;
}

MqttClient::~MqttClient()
{
    stop();
}

void MqttClient::start()
{
    d->client = new QMqttClient(this);
    d->client->setHostname(d->config->mqttHost());
    d->client->setPort(d->config->mqttPort());
    if (!d->config->mqttUsername().isEmpty()) {
        d->client->setUsername(d->config->mqttUsername());
        d->client->setPassword(d->config->mqttPassword());
    }
    connect(d->client, &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState state){
        switch(state) {
            case QMqttClient::Disconnected:
                qWarning() << "Forcibly disconnected from the MQTT broker";
                stop();
                break;
            case QMqttClient::Connecting:
                qDebug() << "Connecting to MQTT broker...";
                break;
            case QMqttClient::Connected:
                for (const QString &topic : d->config->toggleTopics()) {
                    d->handleSubscription(d->client->subscribe(topic, 1));
                }
                break;
        }
    });
    d->client->connectToHost();
}

void MqttClient::stop()
{
    d->client->deleteLater();
    d->client = nullptr;
    d->subscriptions.clear();
}

void MqttClient::restart()
{
    stop();
    start();
}

InputHandler *MqttClient::inputHandler() const
{
    return d->inputHandler;
}
