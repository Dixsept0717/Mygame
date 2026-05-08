#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <QObject>

enum class ControlMode {
    Mouse,
    Keyboard
};

enum class Difficulty {
    Easy,
    Normal,
    Hard
};

enum class ThemeId {
    Default,
    McDonalds
};

struct DifficultyConfig {
    int spawnIntervalMs = 1400;
    int maxEnemies = 16;
    double enemySpeedMultiplier = 1.0;
    int spawnBatch = 1;
};

inline DifficultyConfig difficultyConfigFor(Difficulty difficulty)
{
    switch (difficulty) {
        case Difficulty::Easy:
            return DifficultyConfig{1800, 10, 0.9, 1};
        case Difficulty::Hard:
            return DifficultyConfig{900, 24, 1.15, 1};
        case Difficulty::Normal:
        default:
            return DifficultyConfig{1400, 16, 1.0, 1};
    }
}

struct GameSession {
    Difficulty difficulty = Difficulty::Normal;
    DifficultyConfig difficultyConfig = difficultyConfigFor(Difficulty::Normal);
    ThemeId activeThemeId = ThemeId::Default;
    int winLevel = 10;
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

    void setPendingThemeId(ThemeId themeId) { m_pendingThemeId = themeId; emit pendingThemeChanged(themeId); }
    ThemeId pendingThemeId() const { return m_pendingThemeId; }

signals:
    void volumeChanged(int vol);
    void controlModeChanged(ControlMode mode);
    void pendingThemeChanged(ThemeId themeId);

private:
    GameConfig() : m_volume(50), m_controlMode(ControlMode::Mouse), m_pendingThemeId(ThemeId::Default) {}
    int m_volume;
    ControlMode m_controlMode;
    ThemeId m_pendingThemeId;
};

#endif // GAMECONFIG_H
