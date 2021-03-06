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

#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <memory>

class ConfigPrivate;
class Config : public QObject
{
    Q_OBJECT
public:
    Config(const QString &configFile, QObject *parent = nullptr);
    ~Config() override;

    bool isValid() const;

    QStringList toggleTopics() const;
    char charForTopic(const QString &topic) const;
    QStringList statusTopics() const;

    QString mqttHost() const;
    int mqttPort() const;
    QString mqttUsername() const;
    QString mqttPassword() const;
private:
    std::unique_ptr<ConfigPrivate> d;
};

#endif//CONFIG_H
