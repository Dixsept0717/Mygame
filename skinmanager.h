#ifndef SKINMANAGER_H
#define SKINMANAGER_H

#include "gameconfig.h"

#include <QHash>
#include <QPixmap>

enum class FishVisualType {
    Player,
    Enemy
};

class SkinManager {
public:
    static SkinManager& instance();

    QPixmap pixmapFor(ThemeId themeId, FishVisualType type, int level, int variant = 1);
    void warmUpTheme(ThemeId themeId, int maxLevel = 10);
    void clearCache();

private:
    SkinManager() = default;
    QString cacheKey(ThemeId themeId, FishVisualType type, int level, int variant) const;
    QPixmap generatePlaceholder(ThemeId themeId, FishVisualType type, int level, int variant) const;

    QHash<QString, QPixmap> m_cache;
};

#endif // SKINMANAGER_H

