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

#include "config.h"

#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

static const QHash<int, char> positionTranslator{
    {0, '1'},
    {1, '2'},
    {2, '3'},
    {3, '4'},
    {4, '5'},
    {5, '6'},
    {6, '7'},
    {7, '8'}
};

class ConfigPrivate
{
public:
    ConfigPrivate() {}
    QString configFile;
    bool isValid{false};
    QStringList toggleTopics;
    QStringList statusTopics;
    QString mqttHost;
    int mqttPort{1883};
    QString mqttUsername;
    QString mqttPassword;
};

Config::Config(const QString &configFile, QObject *parent)
    : QObject(parent)
    , d(new ConfigPrivate)
{
    d->configFile = configFile;
    KConfig configReader(configFile, KConfig::SimpleConfig);
    qDebug() << "Reading config from" << configFile;
    if (configReader.hasGroup("General")) {
        const KConfigGroup generalGroup = configReader.group("General");
        if (generalGroup.hasKey("toggleTopics")) {
            d->toggleTopics = generalGroup.readEntry("toggleTopics", QStringList{});
            qDebug() << "Found toggle topics in the configuration, now set to:" << d->toggleTopics;
        }
        if (generalGroup.hasKey("statusTopics")) {
            d->statusTopics = generalGroup.readEntry("statusTopics", QStringList{});
            qDebug() << "Found status topics in the configuration, now set to:" << d->statusTopics;
        }
        d->mqttHost = generalGroup.readEntry("mqttHost", QString{});
        d->mqttPort = generalGroup.readEntry("mqttPort", 1883);
        d->mqttUsername = generalGroup.readEntry("mqttUsername", QString{});
        d->mqttPassword = generalGroup.readEntry("mqttPassword", QString{});
        qDebug() << "Our MQTT host is" << d->mqttHost << d->mqttPort;

        // Now read from the Topics group
        if (configReader.hasGroup("Topics")) {
            static const QLatin1String pathSeparator{"/"};
            const KConfigGroup topicsGroup = configReader.group("Topics");
            QString topicBase = topicsGroup.readEntry("topicBase", QString());
            if (!topicBase.isEmpty() && !topicBase.endsWith(pathSeparator)) {
                topicBase += pathSeparator;
            }
            QString toggleEndpoint = topicsGroup.readEntry("toggleEndpoint", QString("toggle"));
            if (!toggleEndpoint.isEmpty() && !toggleEndpoint.startsWith(pathSeparator)) {
                toggleEndpoint = pathSeparator + toggleEndpoint;
            }
            QString statusEndpoint = topicsGroup.readEntry("statusEndpoint", QString("status"));
            if (!statusEndpoint.isEmpty() && !statusEndpoint.startsWith(pathSeparator)) {
                statusEndpoint = pathSeparator + statusEndpoint;
            }
            for (int i = 1; i < 9; ++i) {
                const QString topic = topicsGroup.readEntry(QString("topic-%1").arg(QString::number(i)), QString());
                if (topic.isEmpty()) {
                    break;
                }
                d->toggleTopics << QString("%1%2%3").arg(topicBase).arg(topic).arg(toggleEndpoint);
                d->statusTopics << QString("%1%2%3").arg(topicBase).arg(topic).arg(statusEndpoint);
            }
            qDebug() << "Topic section found, set topics to:";
            qDebug() << d->toggleTopics;
            qDebug() << d->statusTopics;
        }

        // Sanity check time - make sure we've got everything filled out that we want filled out
        if (d->toggleTopics.count() > 0 && !d->mqttHost.isEmpty()) {
            d->isValid = true;
        }
    }
}

Config::~Config()
{
}

QStringList Config::toggleTopics() const
{
    return d->toggleTopics;
}

char Config::charForTopic(const QString &topic) const
{
    char result{0};
    const int index = d->toggleTopics.indexOf(topic);
    if (index > -1 && index < positionTranslator.count()) {
        result = positionTranslator.value(index);
    }
    return result;
}

QStringList Config::statusTopics() const
{
    return d->statusTopics;
}

bool Config::isValid() const
{
    return d->isValid;
}

QString Config::mqttHost() const
{
    return d->mqttHost;
}

int Config::mqttPort() const
{
    return d->mqttPort;
}

QString Config::mqttUsername() const
{
    return d->mqttUsername;
}

QString Config::mqttPassword() const
{
    return d->mqttPassword;
}
