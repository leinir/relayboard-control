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

#include "inputhandler.h"

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

class KeyboardThread : public QThread
{
    Q_OBJECT
public:
    KeyboardThread(QObject *parent = nullptr)
        : QThread(parent)
    {
        initTermios(0);
    }
    ~KeyboardThread() override {
        resetTermios();
    }
    void run() override {
        while (true)
        {
            char key = getchar();
            Q_EMIT keyPressed(key);
            if (key == 'q' || key == 'Q') {
                break;
            }
        }
    }
    Q_SIGNAL void keyPressed(char keyValue);
};

class PinReaderThread : public QThread
{
    Q_OBJECT
public:
    PinReaderThread(InputHandler::InputChannel channel, QObject *parent = nullptr)
        : QThread(parent)
        , channel(channel)
    {
        bcm2835_gpio_fsel(channel, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(channel, BCM2835_GPIO_PUD_UP);
    }
    ~PinReaderThread() override {
    }
    void run() override {
        uint8_t value{0};
        while (true) {
            if (shouldAbort) {
                break;
            }
            value = bcm2835_gpio_lev(channel);
            if (value != lastValue) {
                Q_EMIT pinValueChanged(channel, QString::number(value));
            }
            lastValue = value;
            usleep(250);
        }
    }
    Q_SIGNAL void pinValueChanged(InputHandler::InputChannel channel, QString newPinValue);
    Q_SLOT void abort() {
        shouldAbort = true;
    }
    const QString mostRecentValue() const {
        return QString::number(lastValue);
    }
private:
    // Set to max to try and ensure we get updated on the first run
    uint8_t lastValue{UINT8_MAX};
    InputHandler::InputChannel channel;
    bool shouldAbort{false};
};

class InputHandlerPrivate {
public:
    InputHandlerPrivate() {}
    ~InputHandlerPrivate() {
        if (inputThread) {
            inputThread->quit();
            inputThread->wait(1000);
        }
        for (PinReaderThread *pinReader : pinReaders) {
            pinReader->abort();
            pinReader->wait(1000);
        }
        if (bcm2835_close()) {
            qDebug() << "Successfully shut down the relay connection";
        }
    }
    KeyboardThread *inputThread{nullptr};
    QList<PinReaderThread*> pinReaders;
};

InputHandler::InputHandler(QObject *parent)
    : QObject(parent)
    , d(new InputHandlerPrivate)
{
    d->inputThread = new KeyboardThread(this);
    connect(d->inputThread, &KeyboardThread::keyPressed, this, &InputHandler::handleKeyPressed);
    connect(d->inputThread, &QThread::finished, qApp, &QCoreApplication::quit);
    if (bcm2835_init()) {
        bcm2835_gpio_fsel(RelayChannel1, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel2, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel3, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel4, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel5, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel6, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel7, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(RelayChannel8, BCM2835_GPIO_FSEL_OUTP);

        d->pinReaders << new PinReaderThread(InputChannel1, this);
        d->pinReaders << new PinReaderThread(InputChannel2, this);
        d->pinReaders << new PinReaderThread(InputChannel3, this);
        d->pinReaders << new PinReaderThread(InputChannel4, this);
        d->pinReaders << new PinReaderThread(InputChannel5, this);
        d->pinReaders << new PinReaderThread(InputChannel6, this);
        d->pinReaders << new PinReaderThread(InputChannel7, this);
        d->pinReaders << new PinReaderThread(InputChannel8, this);
        for (PinReaderThread *pinReader : d->pinReaders) {
            connect(pinReader, &PinReaderThread::pinValueChanged, this, &InputHandler::inputChannelStateChanged, Qt::QueuedConnection);
            pinReader->start();
        }
        qDebug() << "Successfully set up the relays for output. Send the relay number to pulse it, a to pulse all, or q to quit.";
        d->inputThread->start();
    } else {
        qWarning() << "Failed to set up the relays for output!";
        qApp->quit();
    }

}

InputHandler::~InputHandler() = default;

const char *relayChannelName(InputHandler::RelayChannel channel) {
    static const QMetaEnum theEnum = QMetaEnum::fromType<InputHandler::RelayChannel>();
    return theEnum.valueToKey(channel);
}

InputHandler::RelayChannel InputHandler::channelByNumber(int number) const
{
    static const QMetaEnum theEnum = QMetaEnum::fromType<InputHandler::RelayChannel>();
    RelayChannel channel{RelayChannelInvalid};
    if (number > -1 && number < theEnum.keyCount()) {
        bool ok{false};
        int value = theEnum.keyToValue(QString("RelayChannel%1").arg(QString::number(number)).toLatin1(), &ok);
        if (ok) {
            channel = RelayChannel(value);
        }
    }
    return channel;
}

void InputHandler::pulseRelay(InputHandler::RelayChannel channel) const {
    if (channel == RelayChannelInvalid) {
        qWarning() << "Not pulsing invalid relay!";
    } else {
        qDebug() << "Pulsing" << relayChannelName(channel);
        bcm2835_gpio_write(channel,LOW);
        bcm2835_delay(50);
        bcm2835_gpio_write(channel,HIGH);
        bcm2835_delay(50);
    }
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

QStringList InputHandler::mostRecentChannelStates() const
{
    QStringList states;
    for (PinReaderThread *pinReader : d->pinReaders) {
        states << pinReader->mostRecentValue();
    }
    return states;
}

#include "inputhandler.moc"
