#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QDialog>
#include "gameview.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStartGame();
    void onOpenSettings();
    void onExitGame();
    void onBackToMenu();

private:
    void setupUI();
    QWidget* createMainMenu();
    QWidget* createSettingsMenu();
    QWidget* createGameContainer();
    void startGameWithDifficulty(Difficulty difficulty);

    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    
    // UI Elements
    GameView *gameView;
};
#endif // MAINWINDOW_H
