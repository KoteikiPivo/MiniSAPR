#include "schemawidget.h"
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>
#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qvectornd.h>

SchemaWidget::SchemaWidget(QWidget *parent) : QWidget(parent) {
  setStyleSheet("background-color: black;");
  setMinimumSize(400, 400);

  horizontalScrollBar = new QScrollBar(Qt::Horizontal, this);
  horizontalScrollBar->setVisible(false); // Initially hidden
  horizontalScrollBar->setStyleSheet(
      "QScrollBar:horizontal {"
      "    border: 1px solid #999999;"
      "    background: #333333;"
      "    height: 15px;"
      "    margin: 0px;"
      "}"
      "QScrollBar::handle:horizontal {"
      "    background: #666666;"
      "    min-width: 20px;"
      "    border-radius: 7px;"
      "}"
      "QScrollBar::handle:horizontal:hover {"
      "    background: #888888;"
      "}"
      "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
      "    width: 0px;"
      "}");

  // Connect scrollbar
  connect(horizontalScrollBar, &QScrollBar::valueChanged, this,
          &SchemaWidget::onScrollBarValueChanged);
}

void SchemaWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  updateScrollBar();

  // Position scrollbar at the bottom
  int scrollBarHeight = 15;
  horizontalScrollBar->setGeometry(0, height() - scrollBarHeight, width(),
                                   scrollBarHeight);
}

void SchemaWidget::updateScrollBar() {
  if (barLengths.isEmpty()) {
    horizontalScrollBar->setVisible(false);
    return;
  }

  double totalLength = getTotalLength();
  double visibleWidth = width() - 100; // Account for margins
  double totalPixelWidth = totalLength * scale;

  // Show scrollbar only if content doesn't fit
  bool needsScrollBar = (totalPixelWidth > visibleWidth);
  horizontalScrollBar->setVisible(needsScrollBar);

  if (needsScrollBar) {
    int maxScroll = qMax(0, static_cast<int>(totalPixelWidth - visibleWidth));
    horizontalScrollBar->setMaximum(maxScroll);
    horizontalScrollBar->setPageStep(visibleWidth);

    // Sync scrollbar position with current offset
    horizontalScrollBar->setValue(offsetX);
  } else {
    offsetX = 0; // Reset offset if no scrollbar needed
  }
}

void SchemaWidget::onScrollBarValueChanged(int value) {
  offsetX = value;
  update();
}

void SchemaWidget::wheelEvent(QWheelEvent *event) {
  if (event->modifiers() & Qt::ControlModifier) {
    // Zoom with Ctrl + Wheel
    double zoomFactor = 1.1;
    if (event->angleDelta().y() > 0) {
      scale *= zoomFactor;
    } else {
      scale /= zoomFactor;
    }
    updateScrollBar();
    update();
  } else {
    // Scroll with Wheel - update both offset and scrollbar
    offsetX -= event->angleDelta().y() / 8.0;

    // Convert scrollbar maximum to double for comparison
    double maxOffset = static_cast<double>(horizontalScrollBar->maximum());
    offsetX = qMax(0.0, qMin(offsetX, maxOffset));

    // Update scrollbar position
    horizontalScrollBar->setValue(offsetX);
    update();
  }
  event->accept();
}

void SchemaWidget::updateBars(const QVector<double> &lengths,
                              const QVector<double> &surfaces, bool leftAnchor,
                              bool rightAnchor) {
  barLengths = lengths;
  barSurfaces = surfaces;
  hasLeftAnchor = leftAnchor;
  hasRightAnchor = rightAnchor;

  fitToView();
  update();
}

void SchemaWidget::updateNodeForces(const QVector<double> &forces) {
  nodeForces = forces;
  update();
}

void SchemaWidget::updateBarForces(const QVector<double> &forces) {
  barForces = forces;
  update();
}

void SchemaWidget::fitToView() {
  double totalLength = getTotalLength();
  if (totalLength > 0) {
    int viewWidth = width() - 100; // Leave some margin
    scale = viewWidth / totalLength;
    offsetX = 0; // Reset scroll position
  }
  updateScrollBar();
  update();
}

void SchemaWidget::setScale(double newScale) {
  scale = qMax(0.1, newScale); // Minimum scale to prevent too small
  update();
}

double SchemaWidget::getTotalLength() const {
  double totalLength = 0.0;
  for (double length : barLengths) {
    totalLength += length;
  }
  return totalLength;
}

void SchemaWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event)

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  int widgetWidth = width();
  int widgetHeight = height();

  // Adjust height for scrollbar if visible
  int drawingHeight = widgetHeight;
  if (horizontalScrollBar->isVisible()) {
    drawingHeight -= horizontalScrollBar->height();
  }

  // Clear background
  painter.fillRect(0, 0, widgetWidth, widgetHeight, Qt::black);

  if (barLengths.isEmpty()) {
    // Draw placeholder text when no bars
    painter.setPen(Qt::white);
    painter.drawText(QRect(0, 0, widgetWidth, widgetHeight), Qt::AlignCenter,
                     "Добавьте стержни для отображения схемы");
    return;
  }

  // Check validation conditions
  QStringList validationMessages;

  // Check if all bars have valid lengths
  bool allBarsHaveLength = true;
  for (double length : barLengths) {
    if (length <= 0) {
      allBarsHaveLength = false;
      break;
    }
  }
  if (!allBarsHaveLength) {
    validationMessages << "Добавьте каждому стержню длину";
  }

  // Check if at least one anchor is selected
  if (!hasLeftAnchor && !hasRightAnchor) {
    validationMessages << "Выберите хотя бы одну опору (левую или правую)";
  }

  // If there are validation messages, display them and return
  if (!validationMessages.isEmpty()) {
    painter.setPen(Qt::white);

    // Combine messages with newlines
    QString fullMessage = validationMessages.join("\n");

    // Calculate text position for multi-line text
    QFontMetrics metrics(painter.font());
    QRect textRect =
        metrics.boundingRect(QRect(0, 0, widgetWidth, widgetHeight),
                             Qt::AlignCenter | Qt::TextWordWrap, fullMessage);

    // Center the multi-line text
    int textHeight = textRect.height();
    int startY = (widgetHeight - textHeight) / 2;

    // Draw each line separately for better centering
    int lineHeight = metrics.height();
    for (int i = 0; i < validationMessages.size(); i++) {
      QString line = validationMessages[i];
      QRect lineRect(0, startY + i * lineHeight, widgetWidth, lineHeight);
      painter.drawText(lineRect, Qt::AlignCenter, line);
    }
    return;
  }

  // Calculate total length and scaling
  double totalLength = getTotalLength();
  if (totalLength <= 0)
    return;

  drawCoordinateSystem(painter);

  double currentX =
      50 - offsetX; // Start position with margin and scroll offset

  QVector<double> nodePositions;
  nodePositions.append(currentX); // First node

  // Draw left anchor if checked
  if (hasLeftAnchor) {
    drawAnchor(painter, currentX, true);
  }

  // Draw each bar
  for (int i = 0; i < barLengths.size(); i++) {
    double length = barLengths[i];
    double surface = (i < barSurfaces.size()) ? barSurfaces[i] : 1.0;
    if (surface <= 0)
      surface = 1.0;

    double barWidth = length * scale;
    double endX = currentX + barWidth;

    drawBar(painter, currentX, endX, surface, i + 1);

    // Draw node circles
    painter.setBrush(QBrush(Qt::white));
    painter.setPen(QPen(Qt::lightGray, 1));
    painter.drawEllipse(QPointF(currentX, widgetHeight / 2), 4, 4);

    // Move to next bar position
    currentX = endX;
    nodePositions.append(currentX);
  }

  // Draw right anchor if checked
  if (hasRightAnchor) {
    drawAnchor(painter, currentX, false);
  }

  // Draw final node
  painter.setBrush(QBrush(Qt::white));
  painter.setPen(QPen(Qt::lightGray, 1));
  painter.drawEllipse(QPointF(currentX, widgetHeight / 2), 4, 4);

  drawNodeNumbers(painter, nodePositions);
  drawBarForces(painter, nodePositions);
  drawNodeForces(painter, nodePositions);

  // Draw scale and scroll info
  painter.setPen(Qt::white);
  painter.drawText(10, 20,
                   QString("Масштаб: 1px = %1м").arg(1.0 / scale, 0, 'f', 3));

  // Draw scroll position info if scrollbar is visible
  if (horizontalScrollBar->isVisible()) {
    painter.drawText(10, 40, QString("Позиция: %1px").arg(offsetX, 0, 'f', 1));
  }
}

void SchemaWidget::drawCoordinateSystem(QPainter &painter) {
  int widgetHeight = height();
  int widgetWidth = width();
  int centerY = widgetHeight / 2;

  // Adjust height for scrollbar if visible
  if (horizontalScrollBar->isVisible()) {
    widgetHeight -= horizontalScrollBar->height();
  }

  // Draw dotted X-axis
  painter.setPen(QPen(Qt::gray, 1, Qt::DotLine));
  painter.drawLine(0, centerY, widgetWidth, centerY);

  if (showAxisNumbers) {
    // Draw coordinate markers and labels
    painter.setPen(QPen(Qt::lightGray, 1));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    double startCoord = offsetX / scale;
    double endCoord = (offsetX + widgetWidth) / scale;

    // Dynamically calculate marker interval based on scale
    double markerInterval = calculateMarkerInterval();

    // Ensure we don't have too many or too few markers
    int maxMarkers = 20; // Maximum number of markers to display
    int minMarkers = 5;  // Minimum number of markers to display

    double visibleRange = endCoord - startCoord;
    if (visibleRange / markerInterval > maxMarkers) {
      // Increase interval if too many markers
      while (visibleRange / markerInterval > maxMarkers) {
        markerInterval *= 2.0;
      }
    } else if (visibleRange / markerInterval < minMarkers) {
      // Decrease interval if too few markers
      while (visibleRange / markerInterval < minMarkers &&
             markerInterval > 0.001) {
        markerInterval /= 2.0;
      }
    }

    double firstMarker = ceil(startCoord / markerInterval) * markerInterval;

    for (double coord = firstMarker; coord <= endCoord;
         coord += markerInterval) {
      double x = 50 + (coord * scale) - offsetX;

      // Draw marker line
      painter.drawLine(QPointF(x, centerY - 5), QPointF(x, centerY + 5));

      // Draw coordinate label with appropriate precision
      QString label = formatCoordinate(coord, markerInterval);
      QRectF labelRect(x - 25, centerY + 10, 50, 15);
      painter.drawText(labelRect, Qt::AlignCenter, label);
    }

    // Reset font
    font.setPointSize(9);
    painter.setFont(font);
  }
}

double SchemaWidget::calculateMarkerInterval() const {
  if (scale < 1.0) {
    // Very zoomed out - use large intervals
    return 100.0;
  } else if (scale < 10.0) {
    return 10.0;
  } else if (scale < 50.0) {
    return 5.0;
  } else if (scale < 100.0) {
    return 2.0;
  } else if (scale < 200.0) {
    return 1.0;
  } else if (scale < 500.0) {
    return 0.5;
  } else if (scale < 1000.0) {
    return 0.2;
  } else if (scale < 5000.0) {
    return 0.1;
  } else {
    // Very zoomed in - use very small intervals
    return 0.05;
  }
}

QString SchemaWidget::formatCoordinate(double coord, double interval) const {
  // Format coordinate with appropriate number of decimal places
  if (interval >= 10.0) {
    return QString::number(coord, 'f', 0);
  } else if (interval >= 1.0) {
    return QString::number(coord, 'f', 1);
  } else if (interval >= 0.1) {
    return QString::number(coord, 'f', 2);
  } else if (interval >= 0.01) {
    return QString::number(coord, 'f', 3);
  } else {
    return QString::number(coord, 'f', 4);
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

void SchemaWidget::drawBar(QPainter &painter, double startX, double endX,
                           double surface, int barNumber) {
  int widgetHeight = height();
  int centerY = widgetHeight / 2;

  // Calculate bar thickness based on surface
  double maxSurface = getMaxSurface();
  double normalizedSurface = surface / maxSurface;
  int barThickness = qMax(10, static_cast<int>(normalizedSurface * 100));

  // Draw the bar
  QRectF barRect(startX, centerY - barThickness / 2, endX - startX,
                 barThickness);
  painter.setBrush(QBrush(Qt::transparent));
  painter.setPen(QPen(Qt::white, 2));
  painter.drawRect(barRect);

  if (showBarNumbers) {
    // Draw bar number enclosed in a square
    QString numberText = QString::number(barNumber);

    // Calculate text size to determine square size
    QFontMetrics metrics(painter.font());
    QRect textRect = metrics.boundingRect(numberText);
    int squareSize =
        qMax(textRect.width(), textRect.height()) + 8; // Add padding

    // Calculate square position (center of the bar)
    double squareX = barRect.center().x() - squareSize / 2;
    double squareY = barRect.center().y() - squareSize / 2;

    // Draw white square with black border
    painter.setBrush(QBrush(Qt::transparent));
    painter.setPen(QPen(Qt::magenta, 2));
    painter.drawRect(QRectF(squareX, squareY, squareSize, squareSize));

    // Draw black text inside the square
    painter.setPen(Qt::magenta);
    painter.drawText(QRectF(squareX, squareY, squareSize, squareSize),
                     Qt::AlignCenter, numberText);
  }

  // Draw surface value
  painter.setPen(Qt::white);
  QPointF surfaceTextPos(barRect.center().x(), barRect.top() - 10);
  painter.drawText(surfaceTextPos, QString("A=%1").arg(surface, 0, 'f', 2));
}

void SchemaWidget::drawAnchor(QPainter &painter, double x, bool isLeft) {
  int widgetHeight = height();
  int centerY = widgetHeight / 2;

  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::white, 2));

  // Draw vertical line
  int verticalLength = 40;
  painter.drawLine(QPointF(x, centerY - verticalLength),
                   QPointF(x, centerY + verticalLength));

  // Parameters for diagonal lines
  int lineLength = 12;
  int lineSpacing = 8; // Spacing between lines along the vertical axis
  int totalLines = (verticalLength * 2) / lineSpacing;

  if (isLeft) {
    // Draw diagonal lines all going from bottom-left to top-right (45 degrees)
    for (int i = 0; i < totalLines; i++) {
      double lineY = centerY - verticalLength + i * lineSpacing;
      // Only draw if within the vertical line bounds
      if (lineY >= centerY - verticalLength &&
          lineY <= centerY + verticalLength) {
        painter.drawLine(QPointF(x - lineLength, lineY + lineLength),
                         QPointF(x, lineY));
      }
    }
  } else {
    // Draw diagonal lines all going from top-right to bottom-left (45 degrees)
    for (int i = 0; i < totalLines; i++) {
      double lineY = centerY - verticalLength + i * lineSpacing;
      // Only draw if within the vertical line bounds
      if (lineY >= centerY - verticalLength &&
          lineY <= centerY + verticalLength) {
        painter.drawLine(QPointF(x + lineLength, lineY - lineLength),
                         QPointF(x, lineY));
      }
    }
  }
}

void SchemaWidget::drawNodeNumbers(QPainter &painter,
                                   const QVector<double> &nodePositions) {
  if (!showNodeNumbers)
    return;

  int widgetHeight = height();
  int centerY = widgetHeight / 2;

  // Position the node number circles below the construction
  double circleY = centerY + 80; // Position below the bars

  painter.setRenderHint(QPainter::Antialiasing);

  for (int i = 0; i < nodePositions.size(); i++) {
    double nodeX = nodePositions[i];

    // Draw transparent circle with cyan border
    painter.setBrush(Qt::NoBrush);                // Transparent fill
    painter.setPen(QPen(QColor(0, 255, 255), 2)); // Cyan border

    // Draw circle
    double circleRadius = 12;
    painter.drawEllipse(QPointF(nodeX, circleY), circleRadius, circleRadius);

    // Draw node number in cyan
    painter.setPen(QColor(0, 255, 255)); // Cyan text
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);

    // Center the text in the circle
    QString nodeText = QString::number(i + 1);
    QRectF textRect(nodeX - circleRadius, circleY - circleRadius,
                    circleRadius * 2, circleRadius * 2);
    painter.drawText(textRect, Qt::AlignCenter, nodeText);

    // Reset font for other drawing
    font.setBold(false);
    painter.setFont(font);
  }
}

void SchemaWidget::drawNodeForces(QPainter &painter,
                                  const QVector<double> &nodePositions) {
  if (!showNodeForces || nodeForces.isEmpty())
    return;

  int widgetHeight = height();
  int centerY = widgetHeight / 2;

  painter.setRenderHint(QPainter::Antialiasing);

  for (int i = 0; i < nodePositions.size() && i < nodeForces.size(); i++) {
    double nodeX = nodePositions[i];
    double force = nodeForces[i];

    if (force != 0) {
      drawForceArrow(painter, nodeX, centerY, force);
    }
  }
}

void SchemaWidget::drawForceArrow(QPainter &painter, double x, double y,
                                  double force) {
  // Arrow parameters
  double arrowLength = 40.0; // Base length in pixels
  double arrowWidth = 6.0;   // Width of the arrow shaft
  double headLength = 12.0;  // Length of arrow head
  double headWidth = 16.0;   // Width of arrow head

  // Draw horizontal force arrow
  if (force != 0) {
    double dirX = (force > 0) ? 1.0 : -1.0;
    double currentArrowLength = arrowLength;

    QColor arrowColor =
        (force > 0) ? QColor(255, 100, 100) : QColor(100, 100, 255);

    // Calculate arrow end points for horizontal arrow
    QPointF startPoint(x, y);
    QPointF endPoint(x + dirX * currentArrowLength, y);
    drawSingleForceArrow(painter, startPoint, endPoint, arrowColor, arrowWidth,
                         headLength, headWidth, force);
  }
}

void SchemaWidget::drawSingleForceArrow(QPainter &painter,
                                        const QPointF &startPoint,
                                        const QPointF &endPoint,
                                        const QColor &arrowColor,
                                        double arrowWidth, double headLength,
                                        double headWidth, double forceValue) {
  painter.setBrush(QBrush(arrowColor));
  painter.setPen(QPen(arrowColor, 1));

  // Calculate direction vector
  QPointF direction = endPoint - startPoint;
  double arrowLength =
      sqrt(direction.x() * direction.x() + direction.y() * direction.y());

  if (arrowLength < headLength) {
    // Arrow is too short, just draw a simple line
    painter.drawLine(startPoint, endPoint);
    return;
  }

  // Normalize direction
  QPointF dirNormalized = direction / arrowLength;

  // Calculate shaft end (before the arrowhead)
  QPointF shaftEnd = endPoint - dirNormalized * headLength;

  // Draw arrow shaft as a thick rectangle
  QLineF shaftLine(startPoint, shaftEnd);
  QLineF perpendicular = shaftLine.normalVector();
  perpendicular.setLength(arrowWidth / 2.0);

  // Calculate the four corners of the shaft rectangle
  QPointF perpendicularOffset = perpendicular.p2() - perpendicular.p1();
  QPointF p1 = startPoint + perpendicularOffset;
  QPointF p2 = shaftEnd + perpendicularOffset;
  QPointF p3 = shaftEnd - perpendicularOffset;
  QPointF p4 = startPoint - perpendicularOffset;

  // Create shaft polygon
  QPolygonF shaft;
  shaft << p1 << p2 << p3 << p4;
  painter.drawPolygon(shaft);

  // Draw arrow head
  QLineF headPerpendicular(-dirNormalized.y(), dirNormalized.x(), 0, 0);
  headPerpendicular.setLength(headWidth / 2.0);

  QPointF headPoint1 = shaftEnd + headPerpendicular.p2();
  QPointF headPoint2 = shaftEnd - headPerpendicular.p2();

  // Create head polygon - tip at endPoint, base at headPoint1 and headPoint2
  QPolygonF head;
  head << endPoint << headPoint1 << headPoint2;
  painter.drawPolygon(head);

  // Draw force value label
  painter.setPen(arrowColor);
  QFont font = painter.font();
  font.setPointSize(8);
  font.setBold(true);
  painter.setFont(font);

  // Position label based on arrow direction
  QPointF labelOffset;
  labelOffset = (forceValue > 0) ? QPointF(5, -15) : QPointF(-65, -15);

  QString forceText = QString("%1").arg(forceValue, 0, 'f', 2);
  QRectF textRect(endPoint.x() + labelOffset.x(),
                  endPoint.y() + labelOffset.y(), 60, 15);

  // Adjust text alignment based on force direction
  Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter;
  if (forceValue < 0) {
    alignment = Qt::AlignRight | Qt::AlignVCenter;
  }

  painter.drawText(textRect, alignment, forceText);

  // Reset font
  font.setBold(false);
  font.setPointSize(9);
  painter.setFont(font);
}

void SchemaWidget::drawBarForces(QPainter &painter,
                                 const QVector<double> &nodePositions) {
  if (!showBarForces || barForces.isEmpty())
    return;

  int widgetHeight = height();
  int centerY = widgetHeight / 2;

  painter.setRenderHint(QPainter::Antialiasing);

  for (int i = 0; i < barForces.size() && i < nodePositions.size() - 1; i++) {
    double force = barForces[i];
    double startX = nodePositions[i];
    double endX = nodePositions[i + 1];

    // Draw horizontal force if significant
    if (force != 0) {
      drawHorizontalBarForce(painter, startX, endX, centerY, force);
    }
  }
}

void SchemaWidget::drawHorizontalBarForce(QPainter &painter, double startX,
                                          double endX, double centerY,
                                          double force) {
  // Arrow parameters
  double arrowLength = 15.0;
  double arrowWidth = 3.0;
  double headLength = 6.0;
  double headWidth = 8.0;

  QColor arrowColor =
      (force > 0) ? QColor(255, 100, 100) : QColor(100, 100, 255);
  painter.setBrush(QBrush(arrowColor));
  painter.setPen(QPen(arrowColor, 1));

  // Draw multiple arrows along the entire bar
  double barLength = endX - startX;
  int numArrows =
      qMax(2, static_cast<int>(barLength /
                               30)); // Adjust arrow density based on bar length

  for (int i = 0; i < numArrows; i++) {
    double t = (i + 0.5) / numArrows; // Position along the bar (0 to 1)
    double arrowX = startX + t * barLength;

    // Determine arrow direction
    QPointF arrowStart, arrowEnd;
    if (force > 0) {
      // Positive force - arrows point right
      arrowStart = QPointF(arrowX - arrowLength / 2, centerY);
      arrowEnd = QPointF(arrowX + arrowLength / 2, centerY);
    } else {
      // Negative force - arrows point left
      arrowStart = QPointF(arrowX + arrowLength / 2, centerY);
      arrowEnd = QPointF(arrowX - arrowLength / 2, centerY);
    }

    // Draw arrow shaft
    painter.drawLine(arrowStart, arrowEnd);

    // Draw arrow head
    QLineF arrowLine(arrowStart, arrowEnd);
    QLineF perpendicular = arrowLine.normalVector();
    perpendicular.setLength(headWidth / 2.0);

    // Calculate head base
    QLineF directionLine(arrowStart, arrowEnd);
    directionLine.setLength(headLength);
    QPointF headBase = arrowEnd - directionLine.p2() + directionLine.p1();

    QPointF headPoint1 = headBase + perpendicular.p2() - perpendicular.p1();
    QPointF headPoint2 = headBase - (perpendicular.p2() - perpendicular.p1());

    QPolygonF head;
    head << arrowEnd << headPoint1 << headPoint2;
    painter.drawPolygon(head);
  }

  // Draw force label with colored box
  painter.setPen(Qt::white);
  QFont font = painter.font();
  font.setPointSize(8);
  font.setBold(true);
  painter.setFont(font);

  QString forceText = QString("%1").arg(qAbs(force), 0, 'f', 2);

  // Calculate text size to determine box size
  QFontMetrics metrics(font);
  QRect textRect = metrics.boundingRect(forceText);
  int boxWidth = textRect.width() + 8; // Add padding
  int boxHeight = textRect.height() + 4;
  double centerX = (startX + endX) / 2;

  // Determine label position based on force direction
  QPointF labelPos;
  if (force > 0) {
    labelPos = QPointF(centerX - 10, centerY - boxHeight / 2 - 30);
  } else {
    labelPos = QPointF(centerX - boxWidth - 5, centerY - boxHeight / 2 - 30);
  }

  // Draw black-filled box with colored border
  QRectF boxRect(labelPos.x(), labelPos.y(), boxWidth, boxHeight);
  painter.setBrush(QBrush(Qt::black));
  painter.setPen(QPen(arrowColor, 1));
  painter.drawRect(boxRect);

  // Draw text in white inside the box
  painter.setPen(arrowColor);
  painter.drawText(boxRect, Qt::AlignCenter, forceText);

  font.setBold(false);
  font.setPointSize(9);
  painter.setFont(font);
}
