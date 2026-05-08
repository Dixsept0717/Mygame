#include "fish.h"
#include "skinmanager.h"
#include <QtMath>
#include <QRandomGenerator>

Fish::Fish(int level, FishVisualType visualType, ThemeId themeId, int variant, QGraphicsItem *parent)
    : QGraphicsItem(parent),
      m_level(level),
      m_visualType(visualType),
      m_themeId(themeId),
      m_variant(variant)
{
    m_size = 20.0 + level * 10.0;
    updatePixmap();
}

QRectF Fish::boundingRect() const
{
    if (m_pixmap.isNull()) {
        return QRectF(-m_size/2, -m_size/2, m_size, m_size);
    }
    return QRectF(-m_pixmap.width() / 2.0, -m_pixmap.height() / 2.0, m_pixmap.width(), m_pixmap.height());
}

void Fish::setLevel(int level)
{
    if (m_level == level) return;
    prepareGeometryChange();
    m_level = level;
    m_size = 20.0 + m_level * 10.0;
    updatePixmap();
}

void Fish::setThemeId(ThemeId themeId)
{
    if (m_themeId == themeId) return;
    m_themeId = themeId;
    updatePixmap();
}

void Fish::setVariant(int variant)
{
    if (m_variant == variant) return;
    m_variant = variant;
    updatePixmap();
}

void Fish::updatePixmap()
{
    m_pixmap = SkinManager::instance().pixmapFor(m_themeId, m_visualType, m_level, m_variant);
}

void Fish::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    if (!m_pixmap.isNull()) {
        painter->drawPixmap(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap);
        return;
    }
    painter->setBrush(Qt::gray);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(boundingRect());
}

// PlayerFish Implementation
PlayerFish::PlayerFish(ThemeId themeId) : Fish(1, FishVisualType::Player, themeId, 1)
{
    m_speed = 5.0;
    setZValue(10);
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
    setLevel(newLevel);
}

void PlayerFish::reset()
{
    setLevel(1);
    setPos(0, 0);
}

void PlayerFish::setTheme(ThemeId themeId)
{
    setThemeId(themeId);
}

// EnemyFish Implementation
EnemyFish::EnemyFish(int level, ThemeId themeId, double speedMultiplier, bool fromLeft, qreal yPos, int variant)
    : Fish(level, FishVisualType::Enemy, themeId, variant), m_fromLeft(fromLeft)
{
    m_speed = (2.0 + QRandomGenerator::global()->bounded(30) / 10.0) * speedMultiplier;
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
