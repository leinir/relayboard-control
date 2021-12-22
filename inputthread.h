#ifndef INPUTTHREAD_H
#define INPUTTHREAD_H

#include <QObject>
#include <QThread>
class InputThread : public QThread
{
    Q_OBJECT
public:
    InputThread(QObject *parent = nullptr);
    ~InputThread() override;
    void run() override;
signals:
    void keyPressed(char keyValue);
};

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
    InputThread *inputThread{nullptr};
};
 
#endif//INPUTTHREAD_H
