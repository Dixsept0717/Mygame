#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include "gamescene.h"

class GameView : public QGraphicsView {
    Q_OBJECT
public:
    explicit GameView(QWidget *parent = nullptr);
    GameScene* gameScene() const { return m_scene; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    GameScene *m_scene;
};
#endif