#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <QObject>

enum class ControlMode {
    Mouse,
    Keyboard
};

class GameConfig : public QObject {
    Q_OBJECT
public:
    static GameConfig& instance() {
        static GameConfig _instance;
        return _instance;
    }

    void setVolume(int vol) { m_volume = vol; emit volumeChanged(vol); }
    int volume() const { return m_volume; }

    void setControlMode(ControlMode mode) { m_controlMode = mode; emit controlModeChanged(mode); }
    ControlMode controlMode() const { return m_controlMode; }

signals:
    void volumeChanged(int vol);
    void controlModeChanged(ControlMode mode);

private:
    GameConfig() : m_volume(50), m_controlMode(ControlMode::Mouse) {}
    int m_volume;
    ControlMode m_controlMode;
};

#endif // GAMECONFIG_H
