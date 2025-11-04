#ifndef SCHEMAWIDGET_H
#define SCHEMAWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <qcontainerfwd.h>
#include <qscrollbar.h>

class SchemaWidget : public QWidget {
    Q_OBJECT

  public:
    explicit SchemaWidget(QWidget *parent = nullptr);
    void updateBars(const QVector<double> &lengths, const QVector<double> &surfaces,
                    bool leftAnchor, bool rightAnchor);
    void updateNodeForces(const QVector<QVector<double>> &forces);
    void updateBarForces(const QVector<QVector<double>> &forces);
    void fitToView();
    void setScale(double scale);
    double getScale() const { return scale; }

    void setShowNodeNumbers(bool show) {
        showNodeNumbers = show;
        update();
    }
    void setShowBarNumbers(bool show) {
        showBarNumbers = show;
        update();
    }
    void setShowAxisNumbers(bool show) {
        showAxisNumbers = show;
        update();
    }
    void setShowNodeForces(bool show) {
        showNodeForces = show;
        update();
    }
    void setShowBarForces(bool show) {
        showBarForces = show;
        update();
    }
    bool getShowNodeNumbers() const { return showNodeNumbers; }
    bool getShowBarNumbers() const { return showBarNumbers; }
    bool getShowAxisNumbers() const { return showAxisNumbers; }
    bool getShowNodeForces() const { return showNodeForces; }
    bool getShowBarForces() const { return showBarForces; }

  protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private slots:
    void onScrollBarValueChanged(int value);

  private:
    QVector<double> barLengths;
    QVector<double> barSurfaces;
    bool hasLeftAnchor = false;
    bool hasRightAnchor = false;
    double scale = 100.0;
    double offsetX = 0.0;

    QScrollBar *horizontalScrollBar;

    bool showNodeNumbers = true;
    bool showBarNumbers = true;
    bool showAxisNumbers = true;
    bool showNodeForces = true;
    bool showBarForces = true;

    QVector<QVector<double>> nodeForces;
    QVector<QVector<double>> barForces;

    void drawBar(QPainter &painter, double startX, double endX, double surface, int barNumber);
    void drawAnchor(QPainter &painter, double x, bool isLeft);
    void drawNodeNumbers(QPainter &painter, const QVector<double> &nodePositions);
    void drawCoordinateSystem(QPainter &painter);
    void drawNodeForces(QPainter &painter, const QVector<double> &nodePositions);
    void drawForceArrow(QPainter &painter, double x, double y, double forceX, double forceY);
    void drawSingleForceArrow(QPainter &painter, const QPointF &startPoint, const QPointF &endPoint,
                              const QColor &arrowColor, double arrowWidth, double headLength,
                              double headWidth, double forceValue, bool isHorizontal);
    void drawBarForces(QPainter &painter, const QVector<double> &nodePositions);
    void drawHorizontalBarForce(QPainter &painter, double startX, double endX, double centerY,
                                double force);
    void drawVerticalBarForce(QPainter &painter, double startX, double endX, double centerY,
                              double force, double surface);

    void updateScrollBar();
    double calculateMarkerInterval() const;
    QString formatCoordinate(double coord, double interval) const;

    double getMaxSurface() const;
    double getTotalLength() const;
};

#endif // SCHEMAWIDGET_H
