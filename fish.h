#ifndef FISH_H
#define FISH_H

#include <QGraphicsItem>
#include <QPainter>
#include <QObject>
#include <QPointF>

class Fish : public QGraphicsItem {
public:
    Fish(int level, QGraphicsItem *parent = nullptr);
    
    int level() const { return m_level; }
    qreal size() const { return m_size; }
    
    virtual void updatePosition() = 0;
    
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    int m_level;
    qreal m_size;
    QColor m_color;
};

class PlayerFish : public Fish {
public:
    PlayerFish();
    void updatePosition() override;
    void setTargetPos(const QPointF &pos) { m_targetPos = pos; }
    void setKeys(bool up, bool down, bool left, bool right);
    
    void grow(int newLevel);
    void reset();

private:
    QPointF m_targetPos;
    bool m_up = false, m_down = false, m_left = false, m_right = false;
    qreal m_speed = 5.0;
};

class EnemyFish : public Fish {
public:
    EnemyFish(int level, bool fromLeft, qreal yPos);
    void updatePosition() override;
    bool isOffScreen(qreal sceneWidth) const;

private:
    bool m_fromLeft;
    qreal m_speed;
};

#endif // FISH_H
