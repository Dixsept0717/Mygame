#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QTimer>
#include "fish.h"

#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>

class GameScene : public QGraphicsScene {
    Q_OBJECT
public:
    GameScene(QObject *parent = nullptr);
    void startGame(const GameSession& session);
    void stopGame();
    
    PlayerFish* player() const { return m_player; }

signals:
    void gameOver();
    void backToMenuRequested();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();
    void spawnEnemy();

private:
    void checkCollisions();
    void updateHUD();
    void showGameOver();
    void showVictory();
    void showPauseMenu();
    void hidePauseMenu();
    GameSession nextSession() const;

    QTimer *m_gameTimer;
    QTimer *m_spawnTimer;
    PlayerFish *m_player;
    QList<EnemyFish*> m_enemies;
    
    // HUD Items
    QGraphicsTextItem *m_hudText;
    QGraphicsWidget *m_gameOverPanel;
    QGraphicsWidget *m_pausePanel;
    QGraphicsWidget *m_victoryPanel;
    GameSession m_session;
    
    int m_score = 0;
    int m_level = 1;
    int m_exp = 0;
    bool m_isGameOver = false;
    bool m_isPaused = false;

    bool m_keyUp = false, m_keyDown = false, m_keyLeft = false, m_keyRight = false;
};

#endif // GAMESCENE_H
