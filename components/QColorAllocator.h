//
// Created by xtx on 25-6-21.
//

#ifndef QCOLOR_ALLOCATOR_H
#define QCOLOR_ALLOCATOR_H

#include <QColor>
#include  <QHash>
#include <QSet>
#include <QMap>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <cmath>

// 为QColor提供qHash函数重载，使其可以用作QSet和QHash的键
inline size_t qHash(const QColor &color, size_t seed = 0) {
    return qHash(color.rgb(), seed);
}

typedef enum {
    LED_MODE_FLASH_FAST_1 = 1, // 快闪1次
    LED_MODE_FLASH_FAST_2, // 快闪2次
    LED_MODE_FLASH_FAST_3, // 快闪3次
    LED_MODE_FLASH_SLOW, // 慢闪
    LED_MODE_BREATHE_FAST, // 快速呼吸
    LED_MODE_BREATHE_SLOW, // 慢速呼吸
    LED_MODE_STATIC, // 静态显示
} led_mode_t;

class QColorAllocator {
    public:
        QColorAllocator();
        ~QColorAllocator();

        // 分配颜色
        QColor allocateColor(led_mode_t mode);

        // 取消分配颜色
        bool deallocateColor(led_mode_t mode, const QColor &color);

        // 获取指定模式下已分配的颜色
        QSet<QColor> getAllocatedColors(led_mode_t mode) const;

        // 清空指定模式下的所有颜色分配
        void clearMode(led_mode_t mode);

        // 清空所有颜色分配
        void clearAll();

        // 获取可用颜色数量
        int getTotalAvailableColors() const;

    private:
        // 计算两个颜色之间的对比度（基于Lab色彩空间）
        double calculateColorContrast(const QColor &color1, const QColor &color2) const;

        // 将RGB转换为Lab色彩空间
        void rgbToLab(int r, int g, int b, double &L, double &a, double &lab_b) const;

        // 计算颜色与已分配颜色集合的最小对比度
        double calculateMinContrast(const QColor &color, const QSet<QColor> &allocatedColors) const;

        // 选择最佳颜色（优先基础颜色，然后扩展颜色）
        QColor selectBestColor(led_mode_t mode) const;

        // 从指定颜色池中选择最佳颜色
        QColor selectBestColorFromPool(const QVector<QColor> &colorPool, const QSet<QColor> &allocatedColors,
                                       const QSet<QColor> &usedColors) const;

        // 初始化颜色池
        void initializeColorPools();

        // 初始化基础颜色池
        void initializeBaseColors();

        // 初始化扩展颜色池
        void initializeExtendedColors();

    private:
        // 每个模式下已分配的颜色
        QMap<led_mode_t, QSet<QColor> > m_allocatedColors;

        // 基础高对比度颜色池（优先分配）
        QVector<QColor> m_baseColors;

        // 扩展颜色池（基础颜色用完后使用）
        QVector<QColor> m_extendedColors;

        // 线程安全锁
        mutable QMutex m_mutex;

        // 最小对比度阈值
        static constexpr double MIN_CONTRAST_THRESHOLD = 30.0;
};

#endif QCOLOR_ALLOCATOR_H
