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
        RelayChannel1 = 5,
        RelayChannel2 = 6,
        RelayChannel3 = 13,
        RelayChannel4 = 16,
        RelayChannel5 = 19,
        RelayChannel6 = 20,
        RelayChannel7 = 21,
        RelayChannel8 = 26
    };
    Q_ENUM(RelayChannel)
    enum InputChannel {
        InputChannelInvalid = 0,
        InputChannel1 = 1,
        InputChannel2 = 2,
        InputChannel3 = 3,
        InputChannel4 = 4,
        InputChannel5 = 5,
        InputChannel6 = 6,
        InputChannel7 = 7,
        InputChannel8 = 8,
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
