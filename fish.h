#ifndef FISH_H
#define FISH_H

#include <QGraphicsItem>
#include <QPainter>
#include <QObject>
#include <QPointF>

#include "gameconfig.h"
#include "skinmanager.h"

class Fish : public QGraphicsItem {
public:
    Fish(int level, FishVisualType visualType, ThemeId themeId, int variant = 1, QGraphicsItem *parent = nullptr);
    
    int level() const { return m_level; }
    qreal size() const { return m_size; }
    ThemeId themeId() const { return m_themeId; }
    
    virtual void updatePosition() = 0;

    void setLevel(int level);
    void setThemeId(ThemeId themeId);
    void setVariant(int variant);
    
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void updatePixmap();

    int m_level;
    qreal m_size;
    FishVisualType m_visualType;
    ThemeId m_themeId;
    int m_variant;
    QPixmap m_pixmap;
};

class PlayerFish : public Fish {
public:
    explicit PlayerFish(ThemeId themeId = ThemeId::Default);
    void updatePosition() override;
    void setTargetPos(const QPointF &pos) { m_targetPos = pos; }
    void setKeys(bool up, bool down, bool left, bool right);
    
    void grow(int newLevel);
    void reset();
    void setTheme(ThemeId themeId);

private:
    QPointF m_targetPos;
    bool m_up = false, m_down = false, m_left = false, m_right = false;
    qreal m_speed = 5.0;
};

class EnemyFish : public Fish {
public:
    EnemyFish(int level, ThemeId themeId, double speedMultiplier, bool fromLeft, qreal yPos, int variant = 1);
    void updatePosition() override;
    bool isOffScreen(qreal sceneWidth) const;

private:
    bool m_fromLeft;
    qreal m_speed;
};

#endif // FISH_H
