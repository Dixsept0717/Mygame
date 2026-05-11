#include "skinmanager.h"

#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QStringList>

static QStringList skinSearchDirs()
{
    QStringList dirs;
    dirs << QDir::currentPath();
    dirs << QCoreApplication::applicationDirPath();
    dirs << QDir(QCoreApplication::applicationDirPath()).filePath("assets");

    QDir d(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 3; ++i) {
        if (!d.cdUp()) break;
        dirs << d.absolutePath();
        dirs << d.filePath("assets");
    }

    dirs.removeAll(QString());
    dirs.removeDuplicates();
    return dirs;
}

static QString findFirstExistingFile(const QStringList& fileNames)
{
    const QStringList dirs = skinSearchDirs();
    for (const QString& dirPath : dirs) {
        QDir dir(dirPath);
        for (const QString& name : fileNames) {
            const QString path = dir.filePath(name);
            if (QFileInfo::exists(path)) return path;
        }
    }
    return QString();
}

static QSize targetSizeForLevel(int level)
{
    return QSize(70 + level * 42, 42 + level * 24);
}

SkinManager& SkinManager::instance()
{
    static SkinManager inst;
    return inst;
}

QString SkinManager::cacheKey(ThemeId themeId, FishVisualType type, int level, int variant) const
{
    return QString("%1_%2_%3_%4")
        .arg((int)themeId)
        .arg((int)type)
        .arg(level)
        .arg(variant);
}

QPixmap SkinManager::pixmapFor(ThemeId themeId, FishVisualType type, int level, int variant)
{
    const QString key = cacheKey(themeId, type, level, variant);
    auto it = m_cache.find(key);
    if (it != m_cache.end()) return it.value();

    QPixmap pix;
    QStringList names;
    if (themeId == ThemeId::Default) {
        if (type == FishVisualType::Player) {
            names << "Fish_player.png" << "Fish_player.jpg" << "Fish_player.jpeg";
        } else {
            names << QString("Fish_enermy_%1.png").arg(level)
                  << QString("Fish_enermy_%1.jpg").arg(level)
                  << QString("Fish_enermy_%1.jpeg").arg(level);
        }
    } else if (themeId == ThemeId::McDonalds) {
        if (type == FishVisualType::Player) {
            names << "m_player.png" << "m_player.jpg" << "m_player.jpeg";
        } else {
            names << QString("m_enermy_%1.png").arg(level)
                  << QString("m_enermy_%1.jpg").arg(level)
                  << QString("m_enermy_%1.jpeg").arg(level);
        }
    }

    if (!names.isEmpty()) {
        const QString path = findFirstExistingFile(names);
        if (!path.isEmpty()) {
            QPixmap loaded(path);
            if (!loaded.isNull()) {
                const QImage mirrored = loaded.toImage().mirrored(true, false);
                pix = QPixmap::fromImage(mirrored).scaled(targetSizeForLevel(level), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        }
    }

    if (pix.isNull()) {
        pix = generatePlaceholder(themeId, type, level, variant);
    }

    m_cache.insert(key, pix);
    return pix;
}

void SkinManager::warmUpTheme(ThemeId themeId, int maxLevel)
{
    for (int level = 1; level <= maxLevel; ++level) {
        pixmapFor(themeId, FishVisualType::Player, level, 1);
        pixmapFor(themeId, FishVisualType::Enemy, level, 1);
        pixmapFor(themeId, FishVisualType::Enemy, level, 2);
    }
}

void SkinManager::clearCache()
{
    m_cache.clear();
}

QPixmap SkinManager::generatePlaceholder(ThemeId themeId, FishVisualType type, int level, int variant) const
{
    const QSize size = targetSizeForLevel(level);
    const int w = size.width();
    const int h = size.height();
    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QColor primary;
    QColor accent;
    QColor eye = Qt::white;

    if (themeId == ThemeId::Default) {
        primary = (type == FishVisualType::Player) ? QColor(20, 200, 120) : QColor(120, 180, 255);
        accent = (type == FishVisualType::Player) ? QColor(10, 140, 90) : QColor(40, 120, 220);
    } else {
        primary = (type == FishVisualType::Player) ? QColor(255, 215, 0) : QColor(220, 40, 40);
        accent = (type == FishVisualType::Player) ? QColor(255, 165, 0) : QColor(255, 215, 0);
    }

    const int v = qMax(0, (variant - 1) % 3);
    if (v == 1) accent = accent.lighter(125);
    if (v == 2) accent = accent.darker(125);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);

    QRectF bodyRect(0, 0, w * 0.82, h);
    QRectF tailRect(w * 0.78, h * 0.25, w * 0.22, h * 0.5);

    p.setBrush(primary);
    p.drawRoundedRect(bodyRect, h * 0.5, h * 0.5);

    QPolygonF tail;
    tail << QPointF(tailRect.left(), tailRect.top())
         << QPointF(tailRect.right(), tailRect.center().y())
         << QPointF(tailRect.left(), tailRect.bottom());
    p.setBrush(accent);
    p.drawPolygon(tail);

    p.setBrush(accent);
    const qreal stripeW = w * 0.08;
    for (int i = 1; i <= 2; ++i) {
        QRectF stripe(bodyRect.left() + i * stripeW * 2.0, bodyRect.top() + h * 0.15, stripeW, h * 0.7);
        p.setOpacity(0.35);
        p.drawRoundedRect(stripe, 3, 3);
    }
    p.setOpacity(1.0);

    const qreal eyeR = qMax(4.0, h * 0.14);
    QPointF eyeCenter(bodyRect.left() + w * 0.18, bodyRect.top() + h * 0.35);
    p.setBrush(eye);
    p.drawEllipse(eyeCenter, eyeR, eyeR);
    p.setBrush(Qt::black);
    p.drawEllipse(eyeCenter + QPointF(eyeR * 0.2, 0), eyeR * 0.45, eyeR * 0.45);

    QFont font("Arial", qMax(8, h / 4), QFont::Bold);
    p.setFont(font);
    p.setPen(QColor(0, 0, 0, 180));
    p.drawText(QRectF(0, 0, bodyRect.width(), bodyRect.height()), Qt::AlignCenter, QString("Lv%1").arg(level));

    return QPixmap::fromImage(img.mirrored(true, false));
}
