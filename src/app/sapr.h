#ifndef SAPR_H
#define SAPR_H

#include "filehandler.h"
#include "rodsystemcalculator.h"
#include "schemawidget.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QVector>

namespace Ui { class MainWindow; }

class FileHandler;

class Sapr : public QMainWindow {
    Q_OBJECT

public:
    explicit Sapr(QWidget *parent = nullptr);

    // Public methods for FileHandler
    bool getLeftAnchor() const;
    bool getRightAnchor() const;
    int getBarCount() const { return barCount; }
    QString getBarLength(int index) const;
    QString getBarSurface(int index) const;
    QString getBarElasticModulus(int index) const;
    QString getBarTensileStrength(int index) const;
    QVector<double> getAllNodeForces();
    QVector<double> getAllBarForces();
    RodSystemCalculator *calculator;
    bool calculationInProgress;
    QTableWidget *resultsTable;
    QTableWidget *stressTable;

    // Public setters for FileHandler
    void setLeftAnchor(bool anchored);
    void setRightAnchor(bool anchored);
    void clearAllBars();
    void addBar();
    void setBarProperties(int index, const QString &length,
                          const QString &surface, const QString &elasticModulus,
                          const QString &tensileStrength);
    void setNodeForces(const QVector<double> &forces);
    void setBarForces(const QVector<double> &forces);

    // Methods for calculator
    void setupResultsTables();
    void performCalculations();
    void updateResultsTables(const std::vector<double> &displacements,
                             const std::vector<double> &forces,
                             const std::vector<double> &stresses);
    double getSurfaceValue(int index);
    double getElasticModulusValue(int index);
    double getTensileStrengthValue(int index);

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

    QVector<double> savedNodeForces;
    QVector<double> savedBarForces;

    private slots : void on_BarsAdd_clicked();
    void on_LengthEdit_textChanged(const QString &text);
    void on_DeleteButton_clicked();
    void on_action_2_triggered(); // Save
    void on_action_3_triggered(); // Open
    void on_action_5_triggered(); // Exit

private:
    friend class FileHandler;

    QGridLayout *nodeForcesGrid = nullptr;
    QVector<QLabel *> nodeForcesNodeLabels;
    QVector<QLabel *> nodeForcesStartLabels;
    QVector<QLabel *> nodeForcesEndLabels;
    QVector<QLineEdit *> nodeForcesEdits;

    QGridLayout *barForcesGrid = nullptr;
    QVector<QLabel *> barForcesBarLabels;
    QVector<QLineEdit *> barForcesEdits;

    void updateRowNumbers();
    void updateStartPoints();
    double getLengthValue(int index);
    void updateSchemaData();
    void removeBar(int index);

    double getNodeForces(int nodeIndex);
    void updateNodeForces(bool skipSave);
    void setupNodeForcesHeaders();
    void saveNodeForces();
    void loadNodeForces();

    double getBarForces(int barIndex);
    void updateBarForces(bool skipSave);
    void setupBarForcesHeaders();
    void saveBarForces();
    void loadBarForces();

    void applyLoadedForces();
};
#endif
