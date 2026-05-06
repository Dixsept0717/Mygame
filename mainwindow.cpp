#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gameconfig.h"

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

    QPushButton *backBtn = new QPushButton("返回主菜单", widget);
    backBtn->setStyleSheet("font-size: 18px; padding: 10px;");
    layout->addWidget(backBtn);

    connect(volSlider, &QSlider::valueChanged, [](int val){
        GameConfig::instance().setVolume(val);
    });
    connect(ctrlCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [](int index){
        GameConfig::instance().setControlMode(index == 0 ? ControlMode::Mouse : ControlMode::Keyboard);
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

    return widget;
}

void MainWindow::onStartGame()
{
    stackedWidget->setCurrentIndex(2);
    gameView->gameScene()->startGame();
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
    stackedWidget->setCurrentIndex(0);
}
