#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gameconfig.h"
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QRadioButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
        setupUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    stackedWidget->addWidget(createMainMenu());      // Index 0
    stackedWidget->addWidget(createSettingsMenu());  // Index 1
    stackedWidget->addWidget(createGameContainer()); // Index 2

    setWindowTitle("大鱼吃小鱼 - Fish Game");
    resize(1000, 700);
}

QWidget* MainWindow::createMainMenu()
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QLabel *title = new QLabel("大鱼吃小鱼", widget);
    title->setStyleSheet("font-size: 48px; font-weight: bold; color: #006994; margin-bottom: 30px;");
    layout->addWidget(title);

    QPushButton *startBtn = new QPushButton("开始游戏", widget);
    QPushButton *settingsBtn = new QPushButton("游戏设置", widget);
    QPushButton *exitBtn = new QPushButton("退出游戏", widget);

    QString btnStyle = "QPushButton { font-size: 20px; padding: 10px 40px; min-width: 200px; }"
                       "QPushButton:hover { background-color: #e0e0e0; }";
    startBtn->setStyleSheet(btnStyle);
    settingsBtn->setStyleSheet(btnStyle);
    exitBtn->setStyleSheet(btnStyle);

    layout->addWidget(startBtn);
    layout->addWidget(settingsBtn);
    layout->addWidget(exitBtn);

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onOpenSettings);
    connect(exitBtn, &QPushButton::clicked, this, &MainWindow::onExitGame);

    return widget;
}

QWidget* MainWindow::createSettingsMenu()
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QLabel *title = new QLabel("游戏设置", widget);
    title->setStyleSheet("font-size: 32px; font-weight: bold;");
    layout->addWidget(title);

    // 音量设置
    QHBoxLayout *volLayout = new QHBoxLayout();
    volLayout->addWidget(new QLabel("音量: ", widget));
    QSlider *volSlider = new QSlider(Qt::Horizontal, widget);
    volSlider->setRange(0, 100);
    volSlider->setValue(GameConfig::instance().volume());
    volLayout->addWidget(volSlider);
    layout->addLayout(volLayout);

    // 控制模式
    QHBoxLayout *ctrlLayout = new QHBoxLayout();
    ctrlLayout->addWidget(new QLabel("控制模式: ", widget));
    QComboBox *ctrlCombo = new QComboBox(widget);
    ctrlCombo->addItem("鼠标控制");
    ctrlCombo->addItem("键盘控制");
    ctrlCombo->setCurrentIndex(GameConfig::instance().controlMode() == ControlMode::Mouse ? 0 : 1);
    ctrlLayout->addWidget(ctrlCombo);
    layout->addLayout(ctrlLayout);

    // 主题（下一局生效）
    QHBoxLayout *themeLayout = new QHBoxLayout();
    themeLayout->addWidget(new QLabel("主题: ", widget));
    QComboBox *themeCombo = new QComboBox(widget);
    themeCombo->addItem("常规");
    themeCombo->addItem("麦当劳");
    themeCombo->setCurrentIndex(GameConfig::instance().pendingThemeId() == ThemeId::Default ? 0 : 1);
    themeLayout->addWidget(themeCombo);
    layout->addLayout(themeLayout);

    QLabel *themeHint = new QLabel("主题修改：下一局生效", widget);
    themeHint->setStyleSheet("color: #666666; font-size: 12px;");
    layout->addWidget(themeHint);

    QPushButton *backBtn = new QPushButton("返回主菜单", widget);
    backBtn->setStyleSheet("font-size: 18px; padding: 10px;");
    layout->addWidget(backBtn);

    connect(volSlider, &QSlider::valueChanged, [](int val){
        GameConfig::instance().setVolume(val);
    });
    connect(ctrlCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [](int index){
        GameConfig::instance().setControlMode(index == 0 ? ControlMode::Mouse : ControlMode::Keyboard);
    });
    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [](int index){
        GameConfig::instance().setPendingThemeId(index == 0 ? ThemeId::Default : ThemeId::McDonalds);
    });
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::onBackToMenu);

    return widget;
}

QWidget* MainWindow::createGameContainer()
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    gameView = new GameView(widget);
    layout->addWidget(gameView);
    connect(gameView->gameScene(), &GameScene::backToMenuRequested, this, &MainWindow::onBackToMenu);

    return widget;
}

void MainWindow::onStartGame()
{
    QDialog dialog(this);
    dialog.setWindowTitle("选择难度");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *title = new QLabel("请选择游戏难度：", &dialog);
    title->setStyleSheet("font-size: 16px; font-weight: bold;");
    layout->addWidget(title);

    QButtonGroup *group = new QButtonGroup(&dialog);
    QRadioButton *easyBtn = new QRadioButton("简单", &dialog);
    QRadioButton *normalBtn = new QRadioButton("普通", &dialog);
    QRadioButton *hardBtn = new QRadioButton("困难", &dialog);
    normalBtn->setChecked(true);

    group->addButton(easyBtn, (int)Difficulty::Easy);
    group->addButton(normalBtn, (int)Difficulty::Normal);
    group->addButton(hardBtn, (int)Difficulty::Hard);

    layout->addWidget(easyBtn);
    layout->addWidget(normalBtn);
    layout->addWidget(hardBtn);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) return;

    Difficulty chosen = (Difficulty)group->checkedId();
    startGameWithDifficulty(chosen);
}

void MainWindow::startGameWithDifficulty(Difficulty difficulty)
{
    stackedWidget->setCurrentIndex(2);

    GameSession session;
    session.difficulty = difficulty;
    session.difficultyConfig = difficultyConfigFor(difficulty);
    session.activeThemeId = GameConfig::instance().pendingThemeId();
    session.winLevel = 10;

    gameView->gameScene()->startGame(session);
    gameView->setFocus(); // 确保键盘事件能被捕获
}

void MainWindow::onOpenSettings()
{
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onExitGame()
{
    close();
}

void MainWindow::onBackToMenu()
{
    gameView->gameScene()->stopGame();
    gameView->gameScene()->stopBgm();
    stackedWidget->setCurrentIndex(0);
}
