#ifndef INPUTTHREAD_H
#define INPUTTHREAD_H

#include <QObject>
#include <QThread>
class InputThread : public QThread
{
    Q_OBJECT
public:
    InputThread(QObject *parent = 0);
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

    Q_SLOT void handleKeyPressed(char keyValue);
private:
    InputThread *inputThread{nullptr};
};
 
#endif//INPUTTHREAD_H
