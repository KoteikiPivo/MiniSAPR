#include "schemawidget.h"

SchemaWidget::SchemaWidget(QWidget *parent) : QWidget(parent) {
    setStyleSheet("background-color: white;");
}

void SchemaWidget::updateBars(const QVector<double> &lengths, const QVector<double> &surfaces,
                              bool leftAnchor, bool rightAnchor) {
    barLengths = lengths;
    barSurfaces = surfaces;
    hasLeftAnchor = leftAnchor;
    hasRightAnchor = rightAnchor;
    update();
}

void SchemaWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int widgetWidth = width();
    int widgetHeight = height();

    // Clear background
    painter.fillRect(0, 0, widgetWidth, widgetHeight, Qt::white);

    if (barLengths.isEmpty()) {
        // Draw placeholder text when no bars
        painter.setPen(Qt::gray);
        painter.drawText(QRect(0, 0, widgetWidth, widgetHeight), Qt::AlignCenter,
                         "Добавьте стержни для отображения схемы");
        return;
    }

    // Calculate total length and scaling
    double totalLength = 0.0;
    for (double length : barLengths) {
        totalLength += length;
    }

    if (totalLength <= 0)
        return;

    double scaleX = (widgetWidth - 100) / totalLength; // Leave margins for anchors
    double currentX = 50;                              // Start with margin for left anchor

    // Draw left anchor if checked
    if (hasLeftAnchor) {
        drawAnchor(painter, currentX - 25, true);
    }

    // Draw each bar
    for (int i = 0; i < barLengths.size(); i++) {
        double length = barLengths[i];
        double surface = (i < barSurfaces.size()) ? barSurfaces[i] : 1.0;
        if (surface <= 0)
            surface = 1.0;

        double barWidth = length * scaleX;
        double endX = currentX + barWidth;

        drawBar(painter, currentX, endX, surface, i + 1);

        // Draw node circles
        painter.setBrush(QBrush(Qt::red));
        painter.setPen(QPen(Qt::darkRed, 1));
        painter.drawEllipse(QPointF(currentX, widgetHeight / 2), 4, 4);

        // Move to next bar position
        currentX = endX;
    }

    // Draw right anchor if checked
    if (hasRightAnchor) {
        drawAnchor(painter, currentX + 25, false);
    }

    // Draw final node
    painter.setBrush(QBrush(Qt::red));
    painter.setPen(QPen(Qt::darkRed, 1));
    painter.drawEllipse(QPointF(currentX, widgetHeight / 2), 4, 4);

    // Draw scale reference
    painter.setPen(Qt::black);
    if (totalLength > 0) {
        double scaleFactor = totalLength / (widgetWidth - 100);
        painter.drawText(10, 20, QString("Масштаб: 1:%1").arg(scaleFactor, 0, 'f', 0));
    }
}

double SchemaWidget::getMaxSurface() const {
    double maxSurface = 1.0;
    for (double surface : barSurfaces) {
        if (surface > maxSurface) {
            maxSurface = surface;
        }
    }
    return maxSurface > 0 ? maxSurface : 1.0;
}

void SchemaWidget::drawBar(QPainter &painter, double startX, double endX, double surface,
                           int barNumber) {
    int widgetHeight = height();
    int centerY = widgetHeight / 2;

    // Calculate bar thickness based on surface
    double maxSurface = getMaxSurface();
    double normalizedSurface = surface / maxSurface;
    int barThickness = qMax(5, static_cast<int>(normalizedSurface * 50));

    // Draw the bar
    QRectF barRect(startX, centerY - barThickness / 2, endX - startX, barThickness);
    painter.setBrush(QBrush(QColor(70, 130, 180)));
    painter.setPen(QPen(Qt::black, 2));
    painter.drawRect(barRect);

    // Draw bar number
    painter.setPen(Qt::black);
    painter.drawText(barRect.center(), QString::number(barNumber));

    // Draw surface value
    QPointF surfaceTextPos(barRect.center().x(), barRect.top() - 10);
    painter.drawText(surfaceTextPos, QString("A=%1").arg(surface, 0, 'f', 2));
}

void SchemaWidget::drawAnchor(QPainter &painter, double x, bool isLeft) {
    int widgetHeight = height();
    int centerY = widgetHeight / 2;

    painter.setBrush(QBrush(QColor(139, 69, 19)));
    painter.setPen(QPen(Qt::black, 2));

    if (isLeft) {
        QPolygonF triangle;
        triangle << QPointF(x, centerY - 15) << QPointF(x - 20, centerY)
                 << QPointF(x, centerY + 15);
        painter.drawPolygon(triangle);
    } else {
        QPolygonF triangle;
        triangle << QPointF(x, centerY - 15) << QPointF(x + 20, centerY)
                 << QPointF(x, centerY + 15);
        painter.drawPolygon(triangle);
    }
}
