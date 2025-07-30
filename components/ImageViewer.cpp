// ImageViewer.cpp
#include "ImageViewer.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <algorithm>
#include <cmath>
#include <QMainWindow>

ImageViewer::ImageViewer(const QString &imagePath, QWidget *parent, bool fullScreen)
    : QDialog(parent, Qt::FramelessWindowHint | (fullScreen ? Qt::WindowStaysOnTopHint : Qt::Widget))
      , m_scale(1.0)
      , m_offset(0, 0)
      , m_isDragging(false)
      , m_isHoveringCloseButton(false)
      , m_zoomVelocity(0.0) {
    // 加载图片
    m_pixmap.load(imagePath);

    // 设置窗口属性
    setAttribute(Qt::WA_TranslucentBackground);
    // setAttribute(Qt::WA_DeleteOnClose);
    setModal(false); // 设置为模态对话框

    if (fullScreen) {
        // 全屏模式
        QScreen *screen = QApplication::primaryScreen();
        setGeometry(screen->geometry());
    } else if (parent) {
        // 方案1：覆盖整个父窗口（包括标题栏）
        // QPoint globalPos = parent->mapToGlobal(QPoint(0, 0));
        // setGeometry(globalPos.x(), globalPos.y(), parent->width(), parent->height());

        // 方案2：只覆盖父窗口的客户区域（推荐，适用于QMainWindow）
        QWidget *target = parent;

        // 如果父窗口是QMainWindow，使用其centralWidget
        if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parent)) {
            if (mainWindow->centralWidget()) {
                target = mainWindow->centralWidget();
            }
        }

        // 获取目标widget的全局位置和大小
        QPoint globalPos = target->mapToGlobal(QPoint(0, 0));
        setGeometry(globalPos.x(), globalPos.y(), target->width(), target->height());
    } else {
        // 没有父窗口时，使用屏幕的80%大小
        QScreen *screen = QApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int w = screenGeometry.width() * 0.8;
        int h = screenGeometry.height() * 0.8;
        setGeometry((screenGeometry.width() - w) / 2,
                   (screenGeometry.height() - h) / 2,
                   w, h);
    }

    // 计算初始缩放比例，使图片适应窗口
    if (!m_pixmap.isNull()) {
        double scaleX = width() * 0.8 / m_pixmap.width();
        double scaleY = height() * 0.8 / m_pixmap.height();
        m_scale = std::min(scaleX, scaleY);

        // 居中显示
        updateImagePosition();
    }

    // 设置鼠标跟踪
    setMouseTracking(true);

    // 如果有父窗口且非全屏，安装事件过滤器以跟踪父窗口移动
    if (parent && !fullScreen) {
        parent->installEventFilter(this);
    }

    // 初始化惯性定时器
    m_inertiaTimer = new QTimer(this);
    m_inertiaTimer->setInterval(16); // 约60 FPS
    connect(m_inertiaTimer, &QTimer::timeout, this, &ImageViewer::updateInertia);

    // 初始化时间记录器
    m_lastWheelTime.start();
}
void ImageViewer::updatePicture(const QString &imagePath,QWidget *parent) {
    m_pixmap.load(imagePath);

    QWidget *target = parent;

    // 如果父窗口是QMainWindow，使用其centralWidget
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parent)) {
        if (mainWindow->centralWidget()) {
            target = mainWindow->centralWidget();
        }
    }

    // 获取目标widget的全局位置和大小
    QPoint globalPos = target->mapToGlobal(QPoint(0, 0));
    setGeometry(globalPos.x(), globalPos.y(), target->width(), target->height());

    // 计算初始缩放比例，使图片适应窗口
    if (!m_pixmap.isNull()) {
        double scaleX = width() * 0.8 / m_pixmap.width();
        double scaleY = height() * 0.8 / m_pixmap.height();
        m_scale = std::min(scaleX, scaleY);

        // 居中显示
        updateImagePosition();
    }
    repaint(); // 重绘窗口以显示新图片
}
void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (m_pixmap.isNull()) return;

    // 获取鼠标位置
    m_zoomCenter = event->position();

    // 计算滚轮速度
    double delta = event->angleDelta().y() / 120.0;

    // 根据滚轮间隔时间调整速度感知
    qint64 elapsed = m_lastWheelTime.restart();
    double speedMultiplier = 1.0;
    if (elapsed < 50) { // 快速滚动时增加响应
        speedMultiplier = 1.5;
    }

    // 累加速度
    m_zoomVelocity += delta * 0.02 * speedMultiplier;

    // 限制最大速度
    m_zoomVelocity = std::max(-0.1, std::min(0.1, m_zoomVelocity));

    // 如果定时器未运行，启动它
    if (!m_inertiaTimer->isActive()) {
        m_inertiaTimer->start();
    }
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (isPointInCloseButton(event->pos())) {
            // 点击关闭按钮
            close();
        } else if (isClickOnImage(event->pos())) {
            // 在图片上点击，开始拖动
            m_isDragging = true;
            m_lastMousePos = event->pos();
            setCursor(Qt::ClosedHandCursor);
        } else {
            // 在空白区域点击，关闭窗口
            close();
        }
    }
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    // 检查是否悬停在关闭按钮上
    bool wasHovering = m_isHoveringCloseButton;
    m_isHoveringCloseButton = isPointInCloseButton(event->pos());
    if (wasHovering != m_isHoveringCloseButton) {
        update(); // 重绘以更新按钮状态
    }

    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        // 拖动图片
        QPoint delta = event->pos() - m_lastMousePos;
        m_offset += delta;
        m_lastMousePos = event->pos();
        update();
    } else {
        // 更新鼠标样式
        if (m_isHoveringCloseButton) {
            setCursor(Qt::PointingHandCursor);
        } else if (isClickOnImage(event->pos())) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        if (isPointInCloseButton(event->pos())) {
            setCursor(Qt::PointingHandCursor);
        } else if (isClickOnImage(event->pos())) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void ImageViewer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    // 绘制半透明背景
    painter.fillRect(rect(), QColor(0, 0, 0, 180));

    // 绘制图片
    if (!m_pixmap.isNull()) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        QPointF topLeft = getImageTopLeft();
        QSizeF scaledSize(m_pixmap.width() * m_scale, m_pixmap.height() * m_scale);

        painter.drawPixmap(QRectF(topLeft, scaledSize), m_pixmap, m_pixmap.rect());
    }

    // 绘制关闭按钮
    drawCloseButton(painter);
}

void ImageViewer::updateImagePosition()
{
    // 居中显示图片
    m_offset = QPointF(0, 0);
}

QPointF ImageViewer::getImageTopLeft() const
{
    double x = width() / 2.0 - m_pixmap.width() * m_scale / 2.0 + m_offset.x();
    double y = height() / 2.0 - m_pixmap.height() * m_scale / 2.0 + m_offset.y();
    return QPointF(x, y);
}

bool ImageViewer::isClickOnImage(const QPoint &pos) const
{
    if (m_pixmap.isNull()) return false;

    QPointF topLeft = getImageTopLeft();
    QRectF imageRect(topLeft, QSizeF(m_pixmap.width() * m_scale, m_pixmap.height() * m_scale));

    return imageRect.contains(pos);
}

QRect ImageViewer::getCloseButtonRect() const
{
    // 关闭按钮位于右上角，大小为40x40
    return QRect(width() - 60, 20, 40, 40);
}

bool ImageViewer::isPointInCloseButton(const QPoint &pos) const
{
    return getCloseButtonRect().contains(pos);
}

void ImageViewer::drawCloseButton(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);

    QRect buttonRect = getCloseButtonRect();

    // 背景
    QColor bgColor = m_isHoveringCloseButton ? QColor(255, 255, 255, 40) : QColor(255, 255, 255, 20);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawEllipse(buttonRect);

    // 边框
    QPen borderPen(QColor(255, 255, 255, 100), 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(buttonRect);

    // X 符号
    int margin = 12;
    QPoint topLeft = buttonRect.topLeft() + QPoint(margin, margin);
    QPoint topRight = buttonRect.topRight() + QPoint(-margin, margin);
    QPoint bottomLeft = buttonRect.bottomLeft() + QPoint(margin, -margin);
    QPoint bottomRight = buttonRect.bottomRight() + QPoint(-margin, -margin);

    QPen xPen(m_isHoveringCloseButton ? QColor(255, 255, 255, 255) : QColor(255, 255, 255, 200), 2);
    painter.setPen(xPen);
    painter.drawLine(topLeft, bottomRight);
    painter.drawLine(topRight, bottomLeft);
}

void ImageViewer::applyZoom(double factor, const QPointF &center)
{
    // 计算缩放前的图片坐标
    QPointF imageTopLeft = getImageTopLeft();
    QPointF imageMousePos = (center - imageTopLeft) / m_scale;

    // 应用缩放
    m_scale *= factor;

    // 限制缩放范围
    m_scale = std::max(0.1, std::min(10.0, m_scale));

    // 调整偏移量，使鼠标位置的图片内容保持不变
    QPointF newImageTopLeft = center - imageMousePos * m_scale;
    m_offset = newImageTopLeft - QPointF(width() / 2.0 - m_pixmap.width() * m_scale / 2.0,
                                         height() / 2.0 - m_pixmap.height() * m_scale / 2.0);
}

void ImageViewer::updateInertia()
{
    // 应用当前速度
    if (std::abs(m_zoomVelocity) > 0.0001) {
        double factor = 1.0 + m_zoomVelocity;
        applyZoom(factor, m_zoomCenter);

        // 应用摩擦力
        m_zoomVelocity *= 0.92;

        update();
    } else {
        // 速度太小，停止动画
        m_zoomVelocity = 0.0;
        m_inertiaTimer->stop();
    }
}

bool ImageViewer::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent()) {
        if (event->type() == QEvent::Move || event->type() == QEvent::Resize) {
            // 父窗口移动或改变大小时，更新图片查看器的位置和大小
            QWidget *p = qobject_cast<QWidget*>(parent());
            if (p) {
                QWidget *target = p;

                // 如果父窗口是QMainWindow，使用其centralWidget
                if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(p)) {
                    if (mainWindow->centralWidget()) {
                        target = mainWindow->centralWidget();
                    }
                }

                // 更新位置和大小
                QPoint globalPos = target->mapToGlobal(QPoint(0, 0));
                setGeometry(globalPos.x(), globalPos.y(), target->width(), target->height());
            }
        }
    }

    return QDialog::eventFilter(watched, event);
}