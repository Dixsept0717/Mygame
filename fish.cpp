#include "fish.h"
#include "gameconfig.h"
#include <QtMath>
#include <QRandomGenerator>

Fish::Fish(int level, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_level(level)
{
    m_size = 20.0 + level * 10.0;
    
    // 根据等级设置颜色
    switch(level) {
        case 1: m_color = Qt::gray; break;
        case 2: m_color = Qt::yellow; break;
        case 3: m_color = QColor(255, 165, 0); break; // Orange
        case 4: m_color = Qt::red; break;
        case 5: m_color = QColor(128, 0, 128); break; // Purple
        default: m_color = Qt::blue; break;
    }
}

QRectF Fish::boundingRect() const
{
    return QRectF(-m_size/2, -m_size/2, m_size, m_size);
}

void Fish::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(m_color);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(boundingRect());
    
    // 画一个眼睛来指示方向
    painter->setBrush(Qt::white);
    painter->drawEllipse(m_size/4, -m_size/4, m_size/5, m_size/5);
}

// PlayerFish Implementation
PlayerFish::PlayerFish() : Fish(1)
{
    m_color = Qt::green;
    m_speed = 5.0;
    setZValue(10); // 确保玩家在最上层
}

void PlayerFish::updatePosition()
{
    if (GameConfig::instance().controlMode() == ControlMode::Mouse) {
        // 鼠标控制：向目标点移动
        QPointF currentPos = pos();
        QLineF line(currentPos, m_targetPos);
        if (line.length() > 5.0) {
            qreal angle = qAtan2(m_targetPos.y() - currentPos.y(), m_targetPos.x() - currentPos.x());
            qreal dx = qCos(angle) * m_speed;
            qreal dy = qSin(angle) * m_speed;
            
            qreal newX = currentPos.x() + dx;
            qreal newY = currentPos.y() + dy;
            
            // 边界检查
            newX = qBound(0.0, newX, 1000.0);
            newY = qBound(0.0, newY, 700.0);
            
            setPos(newX, newY);
            
            // 转向鼠标
            setRotation(qRadiansToDegrees(angle));
        }
    } else {
        // 键盘控制
        qreal dx = 0, dy = 0;
        if (m_up) dy -= 1;
        if (m_down) dy += 1;
        if (m_left) dx -= 1;
        if (m_right) dx += 1;
        
        if (dx != 0 || dy != 0) {
            // 归一化位移向量并应用速度
            qreal length = qSqrt(dx * dx + dy * dy);
            dx = (dx / length) * m_speed;
            dy = (dy / length) * m_speed;
            
            qreal newX = x() + dx;
            qreal newY = y() + dy;
            
            // 边界检查 (1000x700 场景)
            newX = qBound(0.0, newX, 1000.0);
            newY = qBound(0.0, newY, 700.0);
            
            setPos(newX, newY);
            setRotation(qRadiansToDegrees(qAtan2(dy, dx)));
        }
    }
}

void PlayerFish::setKeys(bool up, bool down, bool left, bool right)
{
    m_up = up; m_down = down; m_left = left; m_right = right;
}

void PlayerFish::grow(int newLevel)
{
    m_level = newLevel;
    m_size = 20.0 + m_level * 10.0; // 同步基础体型
    prepareGeometryChange();
}

void PlayerFish::reset()
{
    m_level = 1;
    m_size = 30.0;
    setPos(0, 0);
}

// EnemyFish Implementation
EnemyFish::EnemyFish(int level, bool fromLeft, qreal yPos)
    : Fish(level), m_fromLeft(fromLeft)
{
    m_speed = 2.0 + QRandomGenerator::global()->bounded(30) / 10.0; // 随机速度 2.0 - 5.0
    if (!m_fromLeft) m_speed = -m_speed;
    
    setPos(fromLeft ? -100 : 1100, yPos);
    if (!m_fromLeft) setRotation(180);
}

void EnemyFish::updatePosition()
{
    setPos(x() + m_speed, y());
}

bool EnemyFish::isOffScreen(qreal sceneWidth) const
{
    if (m_fromLeft) return x() > sceneWidth + 100;
    else return x() < -100;
}
