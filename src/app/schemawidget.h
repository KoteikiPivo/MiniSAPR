#ifndef SCHEMAWIDGET_H
#define SCHEMAWIDGET_H

#include <QWidget>
#include <QPainter>

class SchemaWidget : public QWidget {
    Q_OBJECT

  public:
    explicit SchemaWidget(QWidget *parent = nullptr);
    void updateBars(const QVector<double> &lengths, const QVector<double> &surfaces,
                    bool leftAnchor, bool rightAnchor);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QVector<double> barLengths;
    QVector<double> barSurfaces;
    bool hasLeftAnchor = false;
    bool hasRightAnchor = false;

    void drawBar(QPainter &painter, double startX, double endX, double surface, int barNumber);
    void drawAnchor(QPainter &painter, double x, bool isLeft);
    double getMaxSurface() const;
};

#endif // SCHEMAWIDGET_H
