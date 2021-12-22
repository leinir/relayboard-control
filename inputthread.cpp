#include "inputthread.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMetaEnum>

#include <bcm2835.h>
#include <termios.h>

static struct termios oldSettings;
static struct termios newSettings;

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
    tcgetattr(0, &oldSettings); /* grab old terminal i/o settings */
    newSettings = oldSettings; /* make new settings same as old settings */
    newSettings.c_lflag &= ~ICANON; /* disable buffered i/o */
    newSettings.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &newSettings); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
    tcsetattr(0, TCSANOW, &oldSettings);
}

InputThread::InputThread(QObject *parent)
    : QThread(parent)
{
    initTermios(0);
}

InputThread::~InputThread()
{
    resetTermios();
}

void InputThread::run()
{
    while (true)
    {
        char key = getchar();
        Q_EMIT keyPressed(key);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }
}

InputHandler::InputHandler(QObject *parent)
    : QObject(parent)
{
    inputThread = new InputThread(this);
    connect(inputThread, &InputThread::keyPressed, this, &InputHandler::handleKeyPressed);
    connect(inputThread, &QThread::finished, qApp, &QCoreApplication::quit);
    if (bcm2835_init()) {
        bcm2835_gpio_fsel(RelayChannel1, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel2, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel3, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel4, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel5, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel6, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel7, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel8, BCM2835_GPIO_FSEL_OUTP);
        qDebug() << "Successfully set up the relays for output. Send the relay number to pulse it, a to pulse all, or q to quit.";
        inputThread->start();
    } else {
        qWarning() << "Failed to set up the relays for output!";
        qApp->quit();
    }

}

InputHandler::~InputHandler()
{
    if (inputThread) {
        inputThread->quit();
        inputThread->wait(1000);
    }
    if (bcm2835_close()) {
        qDebug() << "Successfully shut down the relay connection";
    }
}

const char *relayChannelName(InputHandler::RelayChannel channel) {
    QMetaEnum theEnum = QMetaEnum::fromType<InputHandler::RelayChannel>();
    return theEnum.valueToKey(channel);
}

void pulseRelay(InputHandler::RelayChannel channel) {
    qDebug() << "Pulsing" << relayChannelName(channel);
    bcm2835_gpio_write(channel,LOW);
    bcm2835_delay(50);
    bcm2835_gpio_write(channel,HIGH);
    bcm2835_delay(50);
}

void InputHandler::handleKeyPressed(char keyValue)
{
    if (keyValue == 'q' || keyValue == 'Q') {
        qApp->quit();
    } else if (keyValue == 'a' || keyValue == 'A') {
        pulseRelay(RelayChannel1);
        pulseRelay(RelayChannel2);
        pulseRelay(RelayChannel3);
        pulseRelay(RelayChannel4);
        pulseRelay(RelayChannel5);
        pulseRelay(RelayChannel6);
        pulseRelay(RelayChannel7);
        pulseRelay(RelayChannel8);
    } else if (keyValue == '1') {
        pulseRelay(RelayChannel1);
    } else if (keyValue == '2') {
        pulseRelay(RelayChannel2);
    } else if (keyValue == '3') {
        pulseRelay(RelayChannel3);
    } else if (keyValue == '4') {
        pulseRelay(RelayChannel4);
    } else if (keyValue == '5') {
        pulseRelay(RelayChannel5);
    } else if (keyValue == '6') {
        pulseRelay(RelayChannel6);
    } else if (keyValue == '7') {
        pulseRelay(RelayChannel7);
    } else if (keyValue == '8') {
        pulseRelay(RelayChannel8);
    }
}
