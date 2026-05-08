#include "gamescene.h"
#include "gameconfig.h"
#include "skinmanager.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsProxyWidget>

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 1000, 700);
    setBackgroundBrush(QBrush(QColor(0, 105, 148))); // 海洋蓝

    m_player = new PlayerFish(ThemeId::Default);
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
    m_pausePanel = nullptr;
    m_victoryPanel = nullptr;
}

void GameScene::startGame(const GameSession& session)
{
    m_session = session;
    SkinManager::instance().warmUpTheme(m_session.activeThemeId, m_session.winLevel);

    if (m_gameOverPanel) {
        removeItem(m_gameOverPanel);
        delete m_gameOverPanel;
        m_gameOverPanel = nullptr;
    }
    if (m_victoryPanel) {
        removeItem(m_victoryPanel);
        delete m_victoryPanel;
        m_victoryPanel = nullptr;
    }
    hidePauseMenu();

    m_score = 0;
    m_level = 1;
    m_exp = 0;
    m_isGameOver = false;
    m_isPaused = false;
    m_player->setTheme(m_session.activeThemeId);
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

    m_gameTimer->start(16);
    m_spawnTimer->start(m_session.difficultyConfig.spawnIntervalMs);
}

void GameScene::stopGame()
{
    m_gameTimer->stop();
    m_spawnTimer->stop();
}

void GameScene::gameLoop()
{
    if (m_isGameOver || m_isPaused) return;

    m_player->updatePosition();
    const qreal sceneWidth = sceneRect().width();

    for (int i = 0; i < m_enemies.size(); ++i) {
        EnemyFish *enemy = m_enemies[i];
        enemy->updatePosition();
        if (enemy->isOffScreen(sceneWidth)) {
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
    if (m_enemies.size() >= m_session.difficultyConfig.maxEnemies) return;
    bool fromLeft = QRandomGenerator::global()->bounded(2);
    qreal yPos = QRandomGenerator::global()->bounded((int)sceneRect().height());
    
    // 随机等级：基于玩家当前等级动态调整
    int minLvl = qMax(1, m_level - 1);
    int maxLvl = qMin(m_session.winLevel, m_level + 1);
    int level = QRandomGenerator::global()->bounded(minLvl, maxLvl + 1);

    const int variant = QRandomGenerator::global()->bounded(1, 3);
    for (int i = 0; i < m_session.difficultyConfig.spawnBatch; ++i) {
        if (m_enemies.size() >= m_session.difficultyConfig.maxEnemies) break;
        EnemyFish *enemy = new EnemyFish(level, m_session.activeThemeId, m_session.difficultyConfig.enemySpeedMultiplier, fromLeft, yPos, variant);
        addItem(enemy);
        m_enemies.append(enemy);
    }
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

    if (!m_isGameOver && m_level >= m_session.winLevel) {
        stopGame();
        showVictory();
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
        startGame(nextSession());
    });
    
    m_gameOverPanel = addWidget(panel);
    m_gameOverPanel->setPos(sceneRect().width() / 2 - 150, sceneRect().height() / 2 - 100);
    m_gameOverPanel->setZValue(200);
}

void GameScene::showVictory()
{
    if (m_victoryPanel) return;
    QWidget *panel = new QWidget();
    panel->setStyleSheet("background-color: rgba(0, 0, 0, 150); border-radius: 10px; color: white;");
    QVBoxLayout *layout = new QVBoxLayout(panel);

    QLabel *label = new QLabel("游戏胜利");
    label->setStyleSheet("font-size: 36px; font-weight: bold; color: #00ff88;");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    QLabel *scoreLabel = new QLabel(QString("Final Score: %1").arg(m_score));
    scoreLabel->setStyleSheet("font-size: 20px;");
    scoreLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(scoreLabel);

    QPushButton *restartBtn = new QPushButton("重新开始");
    restartBtn->setStyleSheet("padding: 10px; font-size: 18px;");
    layout->addWidget(restartBtn);

    QPushButton *menuBtn = new QPushButton("回到主菜单");
    menuBtn->setStyleSheet("padding: 10px; font-size: 18px;");
    layout->addWidget(menuBtn);

    connect(restartBtn, &QPushButton::clicked, [this](){
        startGame(nextSession());
    });
    connect(menuBtn, &QPushButton::clicked, [this](){
        emit backToMenuRequested();
    });

    m_victoryPanel = addWidget(panel);
    m_victoryPanel->setPos(sceneRect().width() / 2 - 150, sceneRect().height() / 2 - 110);
    m_victoryPanel->setZValue(200);
}

GameSession GameScene::nextSession() const
{
    GameSession session;
    session.difficulty = m_session.difficulty;
    session.difficultyConfig = difficultyConfigFor(m_session.difficulty);
    session.activeThemeId = GameConfig::instance().pendingThemeId();
    session.winLevel = m_session.winLevel;
    return session;
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isPaused || m_isGameOver) return;
    m_player->setTargetPos(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_isGameOver) return;
        if (m_isPaused) {
            hidePauseMenu();
            m_isPaused = false;
            m_keyUp = m_keyDown = m_keyLeft = m_keyRight = false;
            m_player->setKeys(false, false, false, false);
            m_gameTimer->start(16);
            m_spawnTimer->start(m_session.difficultyConfig.spawnIntervalMs);
        } else {
            m_isPaused = true;
            m_keyUp = m_keyDown = m_keyLeft = m_keyRight = false;
            m_player->setKeys(false, false, false, false);
            stopGame();
            showPauseMenu();
        }
        return;
    }
    if (m_isPaused || m_isGameOver) return;
    if (event->key() == Qt::Key_W) m_keyUp = true;
    if (event->key() == Qt::Key_S) m_keyDown = true;
    if (event->key() == Qt::Key_A) m_keyLeft = true;
    if (event->key() == Qt::Key_D) m_keyRight = true;
    m_player->setKeys(m_keyUp, m_keyDown, m_keyLeft, m_keyRight);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if (m_isPaused || m_isGameOver) return;
    if (event->key() == Qt::Key_W) m_keyUp = false;
    if (event->key() == Qt::Key_S) m_keyDown = false;
    if (event->key() == Qt::Key_A) m_keyLeft = false;
    if (event->key() == Qt::Key_D) m_keyRight = false;
    m_player->setKeys(m_keyUp, m_keyDown, m_keyLeft, m_keyRight);
}

void GameScene::showPauseMenu()
{
    if (m_pausePanel) return;

    QWidget *panel = new QWidget();
    panel->setStyleSheet("background-color: rgba(0, 0, 0, 170); border-radius: 10px; color: white;");
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setSpacing(14);

    QLabel *title = new QLabel("暂停");
    title->setStyleSheet("font-size: 32px; font-weight: bold; color: white;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QLabel *hint = new QLabel("按 ESC 继续游戏");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("font-size: 14px; color: #dddddd;");
    layout->addWidget(hint);

    QHBoxLayout *volLayout = new QHBoxLayout();
    volLayout->addWidget(new QLabel("音量: "));
    QSlider *volSlider = new QSlider(Qt::Horizontal);
    volSlider->setRange(0, 100);
    volSlider->setValue(GameConfig::instance().volume());
    volLayout->addWidget(volSlider);
    layout->addLayout(volLayout);

    QHBoxLayout *ctrlLayout = new QHBoxLayout();
    ctrlLayout->addWidget(new QLabel("控制模式: "));
    QComboBox *ctrlCombo = new QComboBox();
    ctrlCombo->addItem("鼠标控制");
    ctrlCombo->addItem("键盘控制");
    ctrlCombo->setCurrentIndex(GameConfig::instance().controlMode() == ControlMode::Mouse ? 0 : 1);
    ctrlLayout->addWidget(ctrlCombo);
    layout->addLayout(ctrlLayout);

    QHBoxLayout *themeLayout = new QHBoxLayout();
    themeLayout->addWidget(new QLabel("主题: "));
    QComboBox *themeCombo = new QComboBox();
    themeCombo->addItem("常规");
    themeCombo->addItem("麦当劳");
    themeCombo->setCurrentIndex(GameConfig::instance().pendingThemeId() == ThemeId::Default ? 0 : 1);
    themeLayout->addWidget(themeCombo);
    layout->addLayout(themeLayout);

    QLabel *themeHint = new QLabel("主题修改：下一局生效");
    themeHint->setAlignment(Qt::AlignCenter);
    themeHint->setStyleSheet("font-size: 12px; color: #dddddd;");
    layout->addWidget(themeHint);

    QPushButton *restartBtn = new QPushButton("重新开始");
    restartBtn->setStyleSheet("padding: 10px; font-size: 18px;");
    layout->addWidget(restartBtn);

    QPushButton *menuBtn = new QPushButton("回到主菜单");
    menuBtn->setStyleSheet("padding: 10px; font-size: 18px;");
    layout->addWidget(menuBtn);

    connect(volSlider, &QSlider::valueChanged, [](int val){
        GameConfig::instance().setVolume(val);
    });
    connect(ctrlCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [](int index){
        GameConfig::instance().setControlMode(index == 0 ? ControlMode::Mouse : ControlMode::Keyboard);
    });
    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [](int index){
        GameConfig::instance().setPendingThemeId(index == 0 ? ThemeId::Default : ThemeId::McDonalds);
    });
    connect(restartBtn, &QPushButton::clicked, [this](){
        startGame(nextSession());
    });
    connect(menuBtn, &QPushButton::clicked, [this](){
        hidePauseMenu();
        m_isPaused = false;
        stopGame();
        emit backToMenuRequested();
    });

    m_pausePanel = addWidget(panel);
    m_pausePanel->setZValue(200);
    m_pausePanel->setPos(sceneRect().width() / 2 - 220, sceneRect().height() / 2 - 170);
}

void GameScene::hidePauseMenu()
{
    if (!m_pausePanel) return;
    removeItem(m_pausePanel);
    delete m_pausePanel;
    m_pausePanel = nullptr;
}
