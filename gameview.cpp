#include "gameview.h"
#include <QMouseEvent>

GameView::GameView(QWidget *parent) : QGraphicsView(parent) {
    m_scene = new GameScene(this);
    setScene(m_scene);
    
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true); // 必须开启鼠标追踪，否则 mouseMoveEvent 只有在按下鼠标时才触发
    
    // 设置视口属性
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

void GameView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}
