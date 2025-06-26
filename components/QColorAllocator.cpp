//
// Created by xtx on 25-6-21.
//

#include "QColorAllocator.h"
// 实现部分
QColorAllocator::QColorAllocator()
{
    initializeColorPools();
}

QColorAllocator::~QColorAllocator()
{
}

void QColorAllocator::initializeColorPools()
{
    initializeBaseColors();
    initializeExtendedColors();
}

void QColorAllocator::initializeBaseColors()
{
    // 预定义高对比度的基础颜色（避免黑色），这些颜色优先分配
    m_baseColors = {
        QColor(255, 0, 0),      // 红色
        QColor(0, 255, 0),      // 绿色
        QColor(0, 0, 255),      // 蓝色
        QColor(255, 255, 0),    // 黄色
        QColor(255, 0, 255),    // 洋红
        QColor(0, 255, 255),    // 青色
        QColor(255, 128, 0),    // 橙色
        QColor(128, 0, 255),    // 紫色
        QColor(255, 192, 203),  // 粉色
        QColor(0, 128, 128),    // 深青色
        QColor(128, 128, 0),    // 橄榄色
        QColor(128, 0, 128),    // 紫红色
        QColor(255, 165, 0),    // 橙红色
        QColor(0, 128, 0),      // 深绿色
        QColor(128, 0, 0),      // 深红色
        QColor(0, 0, 128),      // 深蓝色
        QColor(255, 255, 255),  // 白色
        QColor(128, 128, 128),  // 灰色
        QColor(192, 192, 192),  // 浅灰色
        QColor(64, 64, 64),     // 深灰色
    };
}

void QColorAllocator::initializeExtendedColors()
{
    // 生成扩展颜色池（在HSV空间中生成，一次性生成并缓存）
    m_extendedColors.clear();

    for (int h = 0; h < 360; h += 15) {        // 色相：每15度一个
        for (int s = 40; s <= 100; s += 20) {   // 饱和度：40%, 60%, 80%, 100%
            for (int v = 40; v <= 100; v += 20) { // 明度：40%, 60%, 80%, 100%
                QColor color = QColor::fromHsv(h, (s * 255) / 100, (v * 255) / 100);

                // 避免黑色和非常暗的颜色
                if (color.lightness() > 15) {
                    // 避免与基础颜色重复
                    bool isDuplicate = false;
                    for (const QColor& baseColor : m_baseColors) {
                        if (calculateColorContrast(color, baseColor) < 10.0) {
                            isDuplicate = true;
                            break;
                        }
                    }

                    if (!isDuplicate) {
                        m_extendedColors.append(color);
                    }
                }
            }
        }
    }

    // 按照与白色的对比度排序，确保颜色分配的一致性
    std::sort(m_extendedColors.begin(), m_extendedColors.end(),
              [this](const QColor& a, const QColor& b) {
                  return calculateColorContrast(a, QColor(255, 255, 255)) >
                         calculateColorContrast(b, QColor(255, 255, 255));
              });
}

QColor QColorAllocator::allocateColor(led_mode_t mode)
{
    QMutexLocker locker(&m_mutex);

    QColor selectedColor = selectBestColor(mode);
    m_allocatedColors[mode].insert(selectedColor);

    return selectedColor;
}

bool QColorAllocator::deallocateColor(led_mode_t mode, const QColor& color)
{
    QMutexLocker locker(&m_mutex);

    if (m_allocatedColors.contains(mode)) {
        return m_allocatedColors[mode].remove(color);
    }

    return false;
}

QSet<QColor> QColorAllocator::getAllocatedColors(led_mode_t mode) const
{
    QMutexLocker locker(&m_mutex);

    return m_allocatedColors.value(mode, QSet<QColor>());
}

void QColorAllocator::clearMode(led_mode_t mode)
{
    QMutexLocker locker(&m_mutex);

    m_allocatedColors[mode].clear();
}

void QColorAllocator::clearAll()
{
    QMutexLocker locker(&m_mutex);

    m_allocatedColors.clear();
}

int QColorAllocator::getTotalAvailableColors() const
{
    return m_baseColors.size() + m_extendedColors.size();
}

QColor QColorAllocator::selectBestColor(led_mode_t mode) const
{
    QSet<QColor> allocatedColors = m_allocatedColors.value(mode, QSet<QColor>());

    // 如果没有已分配的颜色，返回第一个基础颜色
    if (allocatedColors.isEmpty()) {
        return m_baseColors.first();
    }

    // 收集所有模式下已使用的颜色（用于避免重复，但允许跨模式重复）
    QSet<QColor> currentModeUsedColors = allocatedColors;

    // 1. 优先尝试从基础颜色池中分配
    QColor bestColor = selectBestColorFromPool(m_baseColors, allocatedColors, currentModeUsedColors);
    if (!currentModeUsedColors.contains(bestColor)) {
        return bestColor;
    }

    // 2. 基础颜色池用完后，从扩展颜色池中分配
    bestColor = selectBestColorFromPool(m_extendedColors, allocatedColors, currentModeUsedColors);
    return bestColor;
}

QColor QColorAllocator::selectBestColorFromPool(const QVector<QColor>& colorPool,
                                                const QSet<QColor>& allocatedColors,
                                                const QSet<QColor>& usedColors) const
{
    QColor bestColor;
    double maxMinContrast = -1.0;
    bool foundUnused = false;

    for (const QColor& candidate : colorPool) {
        bool isUsed = usedColors.contains(candidate);

        // 如果还有未使用的颜色，优先选择未使用的
        if (!foundUnused && !isUsed) {
            foundUnused = true;
            maxMinContrast = -1.0; // 重置，只在未使用的颜色中比较
        } else if (foundUnused && isUsed) {
            continue; // 已经找到未使用的颜色，跳过已使用的
        }

        double minContrast = calculateMinContrast(candidate, allocatedColors);

        if (minContrast > maxMinContrast) {
            maxMinContrast = minContrast;
            bestColor = candidate;
        }
    }

    // 如果没找到合适的颜色，返回池中第一个颜色
    if (maxMinContrast < 0 && !colorPool.isEmpty()) {
        bestColor = colorPool.first();
    }

    return bestColor;
}

double QColorAllocator::calculateMinContrast(const QColor& color, const QSet<QColor>& allocatedColors) const
{
    if (allocatedColors.isEmpty()) {
        return std::numeric_limits<double>::max();
    }

    double minContrast = std::numeric_limits<double>::max();

    for (const QColor& allocated : allocatedColors) {
        double contrast = calculateColorContrast(color, allocated);
        minContrast = std::min(minContrast, contrast);
    }

    return minContrast;
}

double QColorAllocator::calculateColorContrast(const QColor& color1, const QColor& color2) const
{
    // 转换为Lab色彩空间计算对比度
    double L1, a1, b1, L2, a2, b2;

    rgbToLab(color1.red(), color1.green(), color1.blue(), L1, a1, b1);
    rgbToLab(color2.red(), color2.green(), color2.blue(), L2, a2, b2);

    // 计算欧几里得距离
    double deltaL = L1 - L2;
    double deltaA = a1 - a2;
    double deltaB = b1 - b2;

    return std::sqrt(deltaL * deltaL + deltaA * deltaA + deltaB * deltaB);
}

void QColorAllocator::rgbToLab(int r, int g, int b, double& L, double& a, double& lab_b) const
{
    // 简化的RGB到Lab转换（使用近似算法）
    // 首先转换到XYZ色彩空间
    double rNorm = r / 255.0;
    double gNorm = g / 255.0;
    double bNorm = b / 255.0;

    // 伽马校正
    if (rNorm > 0.04045) rNorm = std::pow((rNorm + 0.055) / 1.055, 2.4);
    else rNorm = rNorm / 12.92;

    if (gNorm > 0.04045) gNorm = std::pow((gNorm + 0.055) / 1.055, 2.4);
    else gNorm = gNorm / 12.92;

    if (bNorm > 0.04045) bNorm = std::pow((bNorm + 0.055) / 1.055, 2.4);
    else bNorm = bNorm / 12.92;

    // RGB到XYZ (sRGB色彩空间)
    double X = rNorm * 0.4124564 + gNorm * 0.3575761 + bNorm * 0.1804375;
    double Y = rNorm * 0.2126729 + gNorm * 0.7151522 + bNorm * 0.0721750;
    double Z = rNorm * 0.0193339 + gNorm * 0.1191920 + bNorm * 0.9503041;

    // 标准化到D65光源
    X = X / 0.95047;
    Y = Y / 1.00000;
    Z = Z / 1.08883;

    // XYZ到Lab
    if (X > 0.008856) X = std::pow(X, 1.0/3.0);
    else X = (7.787 * X) + (16.0/116.0);

    if (Y > 0.008856) Y = std::pow(Y, 1.0/3.0);
    else Y = (7.787 * Y) + (16.0/116.0);

    if (Z > 0.008856) Z = std::pow(Z, 1.0/3.0);
    else Z = (7.787 * Z) + (16.0/116.0);

    L = (116.0 * Y) - 16.0;
    a = 500.0 * (X - Y);
    lab_b = 200.0 * (Y - Z);
}
