#include "mqttclient.h"

#include <QDebug>

static const QHash<QString, char> subscriptionTopics{
    {"foxhole/lights/apex-up/toggle", '1'},
    {"foxhole/lights/apex-down/toggle", '2'},
    {"foxhole/lights/tv-up/toggle", '3'},
    {"foxhole/lights/tv-down/toggle", '4'},
    {"foxhole/lights/bedroom-1/toggle", '5'},
    {"foxhole/lights/bedroom-2/toggle", '6'},
    {"foxhole/lights/kitchen/toggle", '7'},
    {"foxhole/lights/dining/toggle", '8'}
};

class Subscription {
public:
    Subscription(MqttClient *q, QMqttSubscription *subscription)
        : q(q)
        , subscription(subscription)
    {
        QObject::connect(subscription, &QMqttSubscription::messageReceived, q, [this,subscription,q](const QMqttMessage &msg){
            qDebug() << "Received message" << msg.payload() << "for topic" << subscription->topic().filter();
            q->inputHandler()->handleKeyPressed(subscriptionTopics[subscription->topic().filter()]);
        });
        QObject::connect(subscription, &QMqttSubscription::stateChanged, q, [this](){});
        QObject::connect(subscription, &QMqttSubscription::qosChanged, q, [this](){});
        qDebug() << "Subscribed to" << subscription->topic().filter() << "with the parent" << q;
    }
    MqttClient* q;
    QMqttSubscription *subscription;
};

class MqttClientPrivate {
public:
    MqttClientPrivate(MqttClient *q)
        : q(q)
    {}
    MqttClient *q;
    InputHandler *inputHandler{nullptr};;

    QMqttClient *client{nullptr};
    QList<Subscription> subscriptions;

    void handleSubscription(QMqttSubscription *sub) {
        if (sub) {
            subscriptions << Subscription(q, sub);
        } else {
            qWarning() << "Could not subscribe! Is the connection valid?";
        }
    }
};

MqttClient::MqttClient(InputHandler *parent)
    : QObject(parent)
    , d(new MqttClientPrivate(this))
{
    d->inputHandler = parent;
}

MqttClient::~MqttClient()
{
    stop();
}

void MqttClient::start()
{
    d->client = new QMqttClient(this);
    d->client->setHostname("mqtt.foxhole.furry.be");
    d->client->setPort(1883);
    d->client->setClientId("relayboard-control");
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
                for (const QString &key : subscriptionTopics.keys()) {
                    d->handleSubscription(d->client->subscribe(key, 1));
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
