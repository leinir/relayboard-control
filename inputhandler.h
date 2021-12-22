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

#ifndef INPUTTHREAD_H
#define INPUTTHREAD_H

#include <QObject>
#include <QThread>
#include <memory>

class InputHandlerPrivate;
class InputHandler : public QObject
{
    Q_OBJECT
public:
    InputHandler(QObject *parent = 0);
    ~InputHandler() override;

    enum RelayChannel {
        RelayChannelInvalid = 0,
        RelayChannel1 = 5,  // Pin 29 - P5
        RelayChannel2 = 6,  // Pin 31 - P6
        RelayChannel3 = 13, // Pin 33 - P13
        RelayChannel4 = 16, // Pin 36 - P16
        RelayChannel5 = 19, // Pin 35 - P19
        RelayChannel6 = 20, // Pin 38 - P20
        RelayChannel7 = 21, // Pin 40 - P21
        RelayChannel8 = 26, // Pin 37 - P26
    };
    Q_ENUM(RelayChannel)
    enum InputChannel {
        InputChannelInvalid = 0,
        InputChannel1 = 2,  // Pin 3  - P2
        InputChannel2 = 3,  // Pin 5  - P3
        InputChannel3 = 4,  // Pin 7  - P4
        InputChannel4 = 17, // Pin 11 - P17
        InputChannel5 = 27, // Pin 13 - P27
        InputChannel6 = 22, // Pin 15 - P22
        InputChannel7 = 10, // Pin 19 - P10
        InputChannel8 = 9,  // Pin 21 - P9
    };
    Q_ENUM(InputChannel)

    InputHandler::RelayChannel channelByNumber(int number) const;
    Q_SLOT void pulseRelay(InputHandler::RelayChannel channel) const;
    Q_SLOT void handleKeyPressed(char keyValue);
    Q_SIGNAL void inputChannelStateChanged(InputHandler::InputChannel channel, const QString& updatedState);
private:
    std::unique_ptr<InputHandlerPrivate> d;
};

#endif//INPUTTHREAD_H
