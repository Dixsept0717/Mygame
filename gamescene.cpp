#include "gamescene.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QGraphicsProxyWidget>

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 1000, 700);
    setBackgroundBrush(QBrush(QColor(0, 105, 148))); // 海洋蓝

    m_player = new PlayerFish();
    addItem(m_player);
    m_player->setPos(500, 350);

    // 初始化 HUD
    m_hudText = new QGraphicsTextItem();
    m_hudText->setDefaultTextColor(Qt::white);
    m_hudText->setFont(QFont("Arial", 16, QFont::Bold));
    m_hudText->setPos(20, 20);
    m_hudText->setZValue(100);
    addItem(m_hudText);
    updateHUD();

    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &GameScene::gameLoop);

    m_spawnTimer = new QTimer(this);
    connect(m_spawnTimer, &QTimer::timeout, this, &GameScene::spawnEnemy);

    m_gameOverPanel = nullptr;
}

void GameScene::startGame()
{
    if (m_gameOverPanel) {
        removeItem(m_gameOverPanel);
        delete m_gameOverPanel;
        m_gameOverPanel = nullptr;
    }

    m_score = 0;
    m_level = 1;
    m_exp = 0;
    m_isGameOver = false;
    m_player->reset();
    m_player->setPos(500, 350);
    m_player->show();
    
    updateHUD();
    
    // 清除现有的敌人
    for (EnemyFish *enemy : m_enemies) {
        removeItem(enemy);
        delete enemy;
    }
    m_enemies.clear();

    m_gameTimer->start(16); // ~60 FPS
    m_spawnTimer->start(1500); // 每1.5秒生成一条鱼
}

void GameScene::stopGame()
{
    m_gameTimer->stop();
    m_spawnTimer->stop();
}

void GameScene::gameLoop()
{
    if (m_isGameOver) return;

    m_player->updatePosition();

    for (int i = 0; i < m_enemies.size(); ++i) {
        EnemyFish *enemy = m_enemies[i];
        enemy->updatePosition();
        if (enemy->isOffScreen(width())) {
            m_enemies.removeAt(i);
            removeItem(enemy);
            delete enemy;
            --i;
        }
    }

    checkCollisions();
}

void GameScene::spawnEnemy()
{
    bool fromLeft = QRandomGenerator::global()->bounded(2);
    qreal yPos = QRandomGenerator::global()->bounded((int)height());
    
    // 随机等级：基于玩家当前等级动态调整
    int minLvl = qMax(1, m_level - 1);
    int maxLvl = m_level + 1;
    int level = QRandomGenerator::global()->bounded(minLvl, maxLvl + 1);

    EnemyFish *enemy = new EnemyFish(level, fromLeft, yPos);
    addItem(enemy);
    m_enemies.append(enemy);
}

void GameScene::checkCollisions()
{
    QList<QGraphicsItem*> collidingItems = m_player->collidingItems();
    for (QGraphicsItem *item : collidingItems) {
        EnemyFish *enemy = dynamic_cast<EnemyFish*>(item);
        if (enemy) {
            if (m_player->level() >= enemy->level()) {
                // 玩家吃掉敌人
                m_exp += enemy->level() * 10;
                m_score += enemy->level() * 100;
                
                // 升级逻辑
                if (m_exp >= m_level * 50) {
                    m_level++;
                    m_exp = 0;
                    m_player->grow(m_level);
                }

                updateHUD();
                m_enemies.removeOne(enemy);
                removeItem(enemy);
                delete enemy;
            } else {
                // 玩家被吃
                m_isGameOver = true;
                m_player->hide();
                stopGame();
                showGameOver();
            }
        }
    }
}

void GameScene::updateHUD()
{
    m_hudText->setPlainText(QString("Score: %1\nLevel: %2\nEXP: %3/%4")
                            .arg(m_score)
                            .arg(m_level)
                            .arg(m_exp)
                            .arg(m_level * 50));
}

void GameScene::showGameOver()
{
    QWidget *panel = new QWidget();
    panel->setStyleSheet("background-color: rgba(0, 0, 0, 150); border-radius: 10px; color: white;");
    QVBoxLayout *layout = new QVBoxLayout(panel);
    
    QLabel *label = new QLabel("GAME OVER");
    label->setStyleSheet("font-size: 36px; font-weight: bold; color: red;");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    
    QLabel *scoreLabel = new QLabel(QString("Final Score: %1").arg(m_score));
    scoreLabel->setStyleSheet("font-size: 20px;");
    scoreLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(scoreLabel);
    
    QPushButton *restartBtn = new QPushButton("重新开始");
    restartBtn->setStyleSheet("padding: 10px; font-size: 18px;");
    layout->addWidget(restartBtn);
    
    connect(restartBtn, &QPushButton::clicked, [this](){
        startGame();
    });
    
    m_gameOverPanel = addWidget(panel);
    m_gameOverPanel->setPos(width()/2 - 150, height()/2 - 100);
    m_gameOverPanel->setZValue(200);
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_player->setTargetPos(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) m_keyUp = true;
    if (event->key() == Qt::Key_S) m_keyDown = true;
    if (event->key() == Qt::Key_A) m_keyLeft = true;
    if (event->key() == Qt::Key_D) m_keyRight = true;
    m_player->setKeys(m_keyUp, m_keyDown, m_keyLeft, m_keyRight);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) m_keyUp = false;
    if (event->key() == Qt::Key_S) m_keyDown = false;
    if (event->key() == Qt::Key_A) m_keyLeft = false;
    if (event->key() == Qt::Key_D) m_keyRight = false;
    m_player->setKeys(m_keyUp, m_keyDown, m_keyLeft, m_keyRight);
}
