#ifndef SAPR_H
#define SAPR_H

#include <QMainWindow>
#include <QLabel>
#include <QVector>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include "schemawidget.h"

namespace Ui {
class MainWindow;
}

class Sapr : public QMainWindow {
    Q_OBJECT

  public:
    explicit Sapr(QWidget *parent = nullptr);

  private slots:
    void on_BarsAdd_clicked();
    void on_LengthEdit_textChanged(const QString &text);
    void on_DeleteButton_clicked();

  private:
    Ui::MainWindow *ui;
    int barCount = 0;
    QLabel *headerNumber = nullptr;
    QLabel *headerLength = nullptr;
    QLabel *headerStartPoint = nullptr;
    QLabel *headerAction = nullptr;
    QLabel *headerSurface = nullptr;
    QLabel *headerElasticModulus = nullptr;
    QLabel *headerTensileStrength = nullptr;
    SchemaWidget *schemaWidget = nullptr;

    QVector<QLabel *> numberLabels;
    QVector<QLineEdit *> lengthEdits;
    QVector<QLabel *> startPointLabels;
    QVector<QPushButton *> deleteButtons;
    QVector<QLineEdit *> surfaceEdits;
    QVector<QLineEdit *> elasticModulusEdits;
    QVector<QLineEdit *> tensileStrengthEdits;

    void updateRowNumbers();
    void updateStartPoints();
    double getLengthValue(int index);
    void updateSchemaData();
    void removeBar(int index);
    QGridLayout *nodeForcesGrid = nullptr;
    QVector<QLabel *> nodeForcesNodeLabels;
    QVector<QLabel *> nodeForcesStartLabels;
    QVector<QLabel *> nodeForcesEndLabels;
    QVector<QLineEdit *> nodeForcesHEdits;
    QVector<QLineEdit *> nodeForcesVEdits;

    QVector<double> getNodeForces(int nodeIndex);
    QVector<QVector<double>> getAllNodeForces();
    void updateNodeForces();
    void setupNodeForcesHeaders();

    QGridLayout *barForcesGrid = nullptr;
    QVector<QLabel *> barForcesBarLabels;
    QVector<QLineEdit *> barForcesHEdits;
    QVector<QLineEdit *> barForcesVEdits;

    QVector<double> getBarForces(int nodeIndex);
    QVector<QVector<double>> getAllBarForces();
    void updateBarForces();
    void setupBarForcesHeaders();
};
#endif
