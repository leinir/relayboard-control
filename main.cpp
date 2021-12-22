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
