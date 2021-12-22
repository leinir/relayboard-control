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
