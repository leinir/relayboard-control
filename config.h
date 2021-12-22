
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

    QString mqttHost() const;
    int mqttPort() const;
    QString mqttUsername() const;
    QString mqttPassword() const;
private:
    std::unique_ptr<ConfigPrivate> d;
};

#endif//CONFIG_H
