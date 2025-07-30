#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPointF>
#include <QTimer>
#include <QElapsedTimer>

class ImageViewer : public QDialog
{
    Q_OBJECT

public:
        explicit ImageViewer(const QString &imagePath, QWidget *parent = nullptr, bool fullScreen = false);
    void updatePicture(const QString &imagePath, QWidget *parent);

protected:
        void wheelEvent(QWheelEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
        bool eventFilter(QObject *watched, QEvent *event) override;
    private:
        QPixmap m_pixmap;
        double m_scale;
        QPointF m_offset;
        QPoint m_lastMousePos;
        bool m_isDragging;
        bool m_isHoveringCloseButton;

        // 惯性缩放相关
        QTimer *m_inertiaTimer;
        double m_zoomVelocity;
        QPointF m_zoomCenter;
        QElapsedTimer m_lastWheelTime;

        void updateImagePosition();
        QPointF getImageTopLeft() const;
        bool isClickOnImage(const QPoint &pos) const;
        QRect getCloseButtonRect() const;
        bool isPointInCloseButton(const QPoint &pos) const;
        void drawCloseButton(QPainter &painter);
        void applyZoom(double factor, const QPointF &center);
        void updateInertia();
};

#endif // IMAGEVIEWER_H