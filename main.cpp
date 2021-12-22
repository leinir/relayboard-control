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

#include <QCoreApplication>
#include <QCommandLineParser>

#include "config.h"
#include "inputthread.h"
#include "mqttclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("relayboard-control");
    QCoreApplication::setApplicationVersion("1.0");

    QString configFileLoation{"/etc/relayboard-control.rc"};

    QCommandLineParser parser;
    parser.setApplicationDescription("A service style application which is used to control the relays of a Waveshare 8-channel relay board for Raspberry Pi");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("configurationFile", QString("A configuration file which describes which mqtt topics to use for what, as well as the server details for the mqtt broker. Default is %1").arg(configFileLoation));

    parser.process(app);

    const QStringList args{parser.positionalArguments()};
    if (args.length() > 0) {
        configFileLoation = args.at(0);
    }

    Config config(configFileLoation);

    InputHandler inputHandler;
    MqttClient mqttClient(&config, &inputHandler);
    if (config.isValid()) {
        mqttClient.start();
    } else {
        qWarning() << "Failed to load configuration file. Please install a correctly formatted configuration file into" << configFileLoation << "and try again";
    }

    app.exec();
}
