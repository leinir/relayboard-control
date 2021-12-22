/*
* This file is a part of the relayboard-control project
* Copyright (C) 2021  Dan Leinir Turthra Jensen <admin@leinir.dk
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mqttclient.h"

#include <QDebug>

class Subscription {
public:
    Subscription(MqttClient *q, InputHandler::RelayChannel channel, QMqttSubscription *subscription)
        : q(q)
        , channel(channel)
        , subscription(subscription)
    {
        QObject::connect(subscription, &QMqttSubscription::messageReceived, q, [channel,subscription,q](const QMqttMessage &msg){
            qDebug() << "Received message" << msg.payload() << "for topic" << subscription->topic().filter();
            q->inputHandler()->pulseRelay(channel);
        });
        QObject::connect(subscription, &QMqttSubscription::stateChanged, q, [this](){});
        QObject::connect(subscription, &QMqttSubscription::qosChanged, q, [this](){});
        qDebug() << "Subscribed to" << subscription->topic().filter() << "with the parent" << q;
    }
    MqttClient* q;
    InputHandler::RelayChannel channel;
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

    void handleSubscription(QMqttSubscription *sub, int channelNumber) {
        if (sub) {
            subscriptions << Subscription(q, inputHandler->channelByNumber(channelNumber), sub);
        } else {
            qWarning() << "Could not subscribe! Is the connection valid?";
        }
    }
    void handleInputChannelStateChange(InputHandler::InputChannel channel, const QString& updatedState) {
        static const QMetaEnum theEnum = QMetaEnum::fromType<InputHandler::InputChannel>();
        if (channel != InputHandler::InputChannelInvalid) {
            QString theKey{theEnum.valueToKey(channel)};
            theKey.remove("InputChannel");
            int channelNumber = theKey.toInt();
            if (channelNumber > -1 && channelNumber < config->statusTopics().count()) {
                const QString topicToUpdate{config->statusTopics().value(channelNumber - 1)};
                qDebug() << "Publishing" << updatedState << "to" << topicToUpdate;
                client->publish(topicToUpdate, updatedState.toLatin1(), 0, true);
            }
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
                int channelNumber{1};
                for (const QString &topic : d->config->toggleTopics()) {
                    d->handleSubscription(d->client->subscribe(topic, 1), channelNumber);
                    ++channelNumber;
                }
                connect(d->inputHandler, &InputHandler::inputChannelStateChanged,
                        this, [this](InputHandler::InputChannel channel, const QString& updatedState){
                            d->handleInputChannelStateChange(channel, updatedState);
                        });
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
