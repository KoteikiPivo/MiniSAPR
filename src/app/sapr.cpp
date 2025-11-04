#include "sapr.h"
#include "ui_sapr.h"
#include <QLabel>
#include <QPushButton>
#include <qlabel.h>
#include <qlineedit.h>
#include <qnamespace.h>
#include <QDoubleValidator>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtpreprocessorsupport.h>
#include <QPainter>

Sapr::Sapr(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), barCount(0), headerNumber(nullptr),
      headerLength(nullptr), headerStartPoint(nullptr), headerAction(nullptr),
      nodeForcesGrid(nullptr), barForcesGrid(nullptr), schemaWidget(nullptr) {
    ui->setupUi(this);

    ui->BarsGrid->setVerticalSpacing(2);

    schemaWidget = new SchemaWidget(ui->SchemaTab);
    schemaWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    schemaWidget->setStyleSheet("background-color: white;");

    QVBoxLayout *schemaLayout = new QVBoxLayout(ui->SchemaTab);
    schemaLayout->addWidget(schemaWidget);

    connect(ui->checkBoxLeft, &QCheckBox::toggled, this, [this](bool checked) {
        if (schemaWidget) {
            updateSchemaData();
        }
    });
    connect(ui->checkBoxRight, &QCheckBox::toggled, this, [this](bool checked) {
        if (schemaWidget) {
            updateSchemaData();
        }
    });
}

bool firstAdd = true;
void Sapr::on_BarsAdd_clicked() {
    if (firstAdd & (barCount == 0)) {
        headerNumber = new QLabel("Номер");
        headerLength = new QLabel("Длина (L)");
        headerStartPoint = new QLabel("Начало");
        headerAction = new QLabel(" ");
        headerSurface = new QLabel("Площадь (A)");
        headerElasticModulus = new QLabel("Модуль упр. (E)");
        headerTensileStrength = new QLabel("Прочность (σ)");

        headerNumber->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        headerLength->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        headerStartPoint->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        headerSurface->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        headerElasticModulus->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        headerTensileStrength->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        headerAction->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        ui->BarsGrid->addWidget(headerNumber, 0, 0);
        ui->BarsGrid->addWidget(headerStartPoint, 0, 1);
        ui->BarsGrid->addWidget(headerLength, 0, 2);
        ui->BarsGrid->addWidget(headerSurface, 0, 3);
        ui->BarsGrid->addWidget(headerElasticModulus, 0, 4);
        ui->BarsGrid->addWidget(headerTensileStrength, 0, 5);
        ui->BarsGrid->addWidget(headerAction, 0, 6);

        ui->BarsGrid->setRowStretch(0, 0);

        setupNodeForcesHeaders();
        setupBarForcesHeaders();
        firstAdd = false;
    }

    barCount++;
    int row = barCount;

    QLabel *numberLabel = new QLabel(QString("%1").arg(barCount));
    QLineEdit *lengthEdit = new QLineEdit();
    QLineEdit *surfaceEdit = new QLineEdit();
    QLineEdit *elasticModulusEdit = new QLineEdit();
    QLineEdit *tensileStrengthEdit = new QLineEdit();
    QLabel *startPointLabel = new QLabel("0.0");
    QPushButton *deleteButton = new QPushButton("Удалить");

    QDoubleValidator *lengthValidator = new QDoubleValidator(0.0, 100000.0, 3, lengthEdit);
    lengthEdit->setValidator(lengthValidator);
    lengthEdit->setPlaceholderText("0.0");

    QDoubleValidator *surfaceValidator = new QDoubleValidator(0.0, 100000.0, 3, surfaceEdit);
    surfaceEdit->setValidator(surfaceValidator);
    surfaceEdit->setPlaceholderText("м²");

    QDoubleValidator *elasticModulusValidator =
        new QDoubleValidator(0.0, 100000.0, 3, elasticModulusEdit);
    elasticModulusEdit->setValidator(elasticModulusValidator);
    elasticModulusEdit->setPlaceholderText("Па");

    QDoubleValidator *tensileStrengthValidator =
        new QDoubleValidator(0.0, 100000.0, 3, tensileStrengthEdit);
    tensileStrengthEdit->setValidator(tensileStrengthValidator);
    tensileStrengthEdit->setPlaceholderText("Па");

    deleteButton->setStyleSheet("QPushButton { background-color: #ff6b6b; color: white; border: "
                                "none; padding: 2px 8px; font-size: 11px; }"
                                "QPushButton:hover { background-color: #ff5252; }");
    deleteButton->setMaximumWidth(70);

    numberLabels.append(numberLabel);
    lengthEdits.append(lengthEdit);
    surfaceEdits.append(surfaceEdit);
    elasticModulusEdits.append(elasticModulusEdit);
    tensileStrengthEdits.append(tensileStrengthEdit);
    startPointLabels.append(startPointLabel);
    deleteButtons.append(deleteButton);

    connect(lengthEdit, &QLineEdit::textChanged, this, &Sapr::on_LengthEdit_textChanged);
    connect(deleteButton, &QPushButton::clicked, this, &Sapr::on_DeleteButton_clicked);
    connect(surfaceEdit, &QLineEdit::textChanged, this, &Sapr::updateSchemaData);

    deleteButton->setProperty("rowIndex", row - 1);

    numberLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lengthEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    surfaceEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    elasticModulusEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    tensileStrengthEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    startPointLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    deleteButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    numberLabel->setMinimumHeight(30);
    lengthEdit->setMinimumHeight(30);
    surfaceEdit->setMinimumHeight(30);
    elasticModulusEdit->setMinimumHeight(30);
    tensileStrengthEdit->setMinimumHeight(30);
    startPointLabel->setMinimumHeight(30);
    deleteButton->setMinimumHeight(30);

    ui->BarsGrid->addWidget(numberLabel, row, 0);
    ui->BarsGrid->addWidget(startPointLabel, row, 1);
    ui->BarsGrid->addWidget(lengthEdit, row, 2);
    ui->BarsGrid->addWidget(surfaceEdit, row, 3);
    ui->BarsGrid->addWidget(elasticModulusEdit, row, 4);
    ui->BarsGrid->addWidget(tensileStrengthEdit, row, 5);
    ui->BarsGrid->addWidget(deleteButton, row, 6);

    ui->BarsGrid->setRowStretch(row, 0);

    updateNodeForces();
    updateBarForces();
    updateStartPoints();
    updateSchemaData();
}

void Sapr::on_LengthEdit_textChanged(const QString &text) {
    Q_UNUSED(text)
    updateStartPoints();
    updateSchemaData();
}

void Sapr::on_DeleteButton_clicked() {
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        int index = button->property("rowIndex").toInt();
        removeBar(index);
    }
}

double Sapr::getLengthValue(int index) {
    if (index >= 0 && index < lengthEdits.size()) {
        QString text = lengthEdits[index]->text();
        if (!text.isEmpty()) {
            bool ok;
            double value = text.toDouble(&ok);
            if (ok)
                return value;
        }
    }
    return 0.0;
}

void Sapr::updateStartPoints() {
    double cumulativeLength = 0.0;

    for (int i = 0; i < startPointLabels.size(); ++i) {
        startPointLabels[i]->setText(QString::number(cumulativeLength, 'f', 2));

        cumulativeLength += getLengthValue(i);
    }
}

void Sapr::removeBar(int index) {
    if (index < 0 || index >= numberLabels.size())
        return;

    // Delete the bar widgets
    delete numberLabels[index];
    delete lengthEdits[index];
    delete surfaceEdits[index];
    delete elasticModulusEdits[index];
    delete tensileStrengthEdits[index];
    delete startPointLabels[index];
    delete deleteButtons[index];

    // Remove from vectors
    numberLabels.remove(index);
    lengthEdits.remove(index);
    surfaceEdits.remove(index);
    elasticModulusEdits.remove(index);
    tensileStrengthEdits.remove(index);
    startPointLabels.remove(index);
    deleteButtons.remove(index);

    barCount--;

    // If no bars left, clear headers too
    if (barCount == 0) {
        delete headerNumber;
        delete headerLength;
        delete headerSurface;
        delete headerElasticModulus;
        delete headerTensileStrength;
        delete headerStartPoint;
        delete headerAction;
        headerNumber = nullptr;
        headerLength = nullptr;
        headerSurface = nullptr;
        headerElasticModulus = nullptr;
        headerTensileStrength = nullptr;
        headerStartPoint = nullptr;
        headerAction = nullptr;
        firstAdd = true;

        // Clear the grid completely
        QLayoutItem *child;
        while ((child = ui->BarsGrid->takeAt(0)) != nullptr) {
            if (child->widget()) {
                ui->BarsGrid->removeWidget(child->widget());
                delete child->widget();
            }
            delete child;
        }
    } else {
        QLayoutItem *child;
        while ((child = ui->BarsGrid->takeAt(0)) != nullptr) {
            if (child->widget()) {
                ui->BarsGrid->removeWidget(child->widget());
            }
            delete child;
        }

        ui->BarsGrid->addWidget(headerNumber, 0, 0);
        ui->BarsGrid->addWidget(headerStartPoint, 0, 1);
        ui->BarsGrid->addWidget(headerLength, 0, 2);
        ui->BarsGrid->addWidget(headerSurface, 0, 3);
        ui->BarsGrid->addWidget(headerElasticModulus, 0, 4);
        ui->BarsGrid->addWidget(headerTensileStrength, 0, 5);
        ui->BarsGrid->addWidget(headerAction, 0, 4);

        for (int i = 0; i < numberLabels.size(); i++) {
            int row = i + 1;

            deleteButtons[i]->setProperty("rowIndex", i);

            numberLabels[i]->setText(QString("%1").arg(i + 1));

            ui->BarsGrid->addWidget(numberLabels[i], row, 0);
            ui->BarsGrid->addWidget(startPointLabels[i], row, 1);
            ui->BarsGrid->addWidget(lengthEdits[i], row, 2);
            ui->BarsGrid->addWidget(surfaceEdits[i], row, 3);
            ui->BarsGrid->addWidget(elasticModulusEdits[i], row, 4);
            ui->BarsGrid->addWidget(tensileStrengthEdits[i], row, 5);
            ui->BarsGrid->addWidget(deleteButtons[i], row, 4);

            ui->BarsGrid->setRowStretch(row, 0);
        }
    }
    updateNodeForces();
    updateBarForces();
    updateStartPoints();
    updateSchemaData();
}

void Sapr::setupNodeForcesHeaders() {
    // Only create the layout if it doesn't exist
    if (!nodeForcesGrid) {
        nodeForcesGrid = new QGridLayout(ui->NodeForces);
        nodeForcesGrid->setVerticalSpacing(2);
    }

    // Clear existing layout from NodeForces tab
    QLayoutItem *child;
    while ((child = nodeForcesGrid->takeAt(0)) != nullptr) {
        if (child->widget()) {
            nodeForcesGrid->removeWidget(child->widget());
            delete child->widget();
        }
        delete child;
    }

    // Add headers for NodeForces tab
    QLabel *nodeHeader = new QLabel("Узел");
    QLabel *startOfHeader = new QLabel("Начало стержня");
    QLabel *endOfHeader = new QLabel("Конец стержня");
    QLabel *horizontalHeader = new QLabel("Fx");
    QLabel *verticalHeader = new QLabel("Fy");

    nodeHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    startOfHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    endOfHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    horizontalHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    verticalHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Add headers to grid
    nodeForcesGrid->addWidget(nodeHeader, 0, 0);
    nodeForcesGrid->addWidget(startOfHeader, 0, 1);
    nodeForcesGrid->addWidget(endOfHeader, 0, 2);
    nodeForcesGrid->addWidget(horizontalHeader, 0, 3);
    nodeForcesGrid->addWidget(verticalHeader, 0, 4);
}
void Sapr::updateNodeForces() {
    // Safety check
    if (!nodeForcesGrid)
        return;

    // Clear existing node force widgets (keep headers which are at row 0)
    for (auto label : nodeForcesNodeLabels) {
        if (label)
            delete label;
    }
    for (auto startLabel : nodeForcesStartLabels) {
        if (startLabel)
            delete startLabel;
    }
    for (auto endLabel : nodeForcesEndLabels) {
        if (endLabel)
            delete endLabel;
    }
    for (auto edit : nodeForcesHEdits) {
        if (edit)
            delete edit;
    }
    for (auto edit : nodeForcesVEdits) {
        if (edit)
            delete edit;
    }

    nodeForcesNodeLabels.clear();
    nodeForcesStartLabels.clear();
    nodeForcesEndLabels.clear();
    nodeForcesHEdits.clear();
    nodeForcesVEdits.clear();

    // If no bars, clear everything except headers and return
    if (barCount == 0) {
        // Remove all rows except header (row 0)
        for (int row = nodeForcesGrid->rowCount() - 1; row >= 1; row--) {
            for (int col = 0; col < nodeForcesGrid->columnCount(); col++) {
                QLayoutItem *item = nodeForcesGrid->itemAtPosition(row, col);
                if (item && item->widget()) {
                    nodeForcesGrid->removeWidget(item->widget());
                    delete item->widget();
                }
            }
        }
        return;
    }

    // Remove all rows except header (row 0)
    for (int row = nodeForcesGrid->rowCount() - 1; row >= 1; row--) {
        for (int col = 0; col < nodeForcesGrid->columnCount(); col++) {
            QLayoutItem *item = nodeForcesGrid->itemAtPosition(row, col);
            if (item && item->widget()) {
                nodeForcesGrid->removeWidget(item->widget());
                delete item->widget();
            }
        }
    }

    // Add node force inputs, starting from row 1
    // n bars → n+1 nodes
    for (int i = 0; i <= barCount; i++) {
        int row = i + 1;

        // Node label
        QLabel *nodeLabel = new QLabel(QString("%1").arg(i + 1));
        nodeForcesNodeLabels.append(nodeLabel);

        // Start of bar label
        QLabel *startLabel = new QLabel();
        if (i < barCount) {
            startLabel->setText(QString("Стержень %1").arg(i + 1));
        } else {
            startLabel->setText("—"); // Last node doesn't start any bar
        }

        // End of bar label
        QLabel *endLabel = new QLabel();
        if (i > 0) {
            endLabel->setText(QString("Стержень %1").arg(i));
        } else {
            endLabel->setText("—"); // First node doesn't end any bar
        }

        // Force input fields with validators
        QLineEdit *hEdit = new QLineEdit();
        QLineEdit *vEdit = new QLineEdit();

        // Set up validators and placeholders
        QDoubleValidator *validator = new QDoubleValidator(-100000.0, 100000.0, 3, hEdit);
        hEdit->setValidator(validator);
        vEdit->setValidator(new QDoubleValidator(-100000.0, 100000.0, 3, vEdit));

        hEdit->setPlaceholderText("0.0");
        vEdit->setPlaceholderText("0.0");

        // Store references
        nodeForcesStartLabels.append(startLabel);
        nodeForcesEndLabels.append(endLabel);
        nodeForcesHEdits.append(hEdit);
        nodeForcesVEdits.append(vEdit);

        // Add to grid
        nodeForcesGrid->addWidget(nodeLabel, row, 0);
        nodeForcesGrid->addWidget(startLabel, row, 1);
        nodeForcesGrid->addWidget(endLabel, row, 2);
        nodeForcesGrid->addWidget(hEdit, row, 3);
        nodeForcesGrid->addWidget(vEdit, row, 4);
    }
}

void Sapr::setupBarForcesHeaders() {
    // Only create the layout if it doesn't exist
    if (!barForcesGrid) {
        barForcesGrid = new QGridLayout(ui->BarForces);
        barForcesGrid->setVerticalSpacing(2);
        barForcesGrid->setHorizontalSpacing(5);
    }

    // Clear existing layout from BarForces tab
    QLayoutItem *child;
    while ((child = barForcesGrid->takeAt(0)) != nullptr) {
        if (child->widget()) {
            barForcesGrid->removeWidget(child->widget());
            delete child->widget();
        }
        delete child;
    }

    // Add headers for BarForces tab
    QLabel *barHeader = new QLabel("Стержень");
    QLabel *horizontalHeader = new QLabel("Fx");
    QLabel *verticalHeader = new QLabel("Fy");

    barHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    horizontalHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    verticalHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Add headers to grid
    barForcesGrid->addWidget(barHeader, 0, 0);
    barForcesGrid->addWidget(horizontalHeader, 0, 1);
    barForcesGrid->addWidget(verticalHeader, 0, 2);
}

void Sapr::updateBarForces() {
    // Safety check
    if (!barForcesGrid)
        return;

    // Clear existing bar force widgets (keep headers which are at row 0)
    for (auto label : barForcesBarLabels) {
        if (label)
            delete label;
    }
    for (auto edit : barForcesHEdits) {
        if (edit)
            delete edit;
    }
    for (auto edit : barForcesVEdits) {
        if (edit)
            delete edit;
    }

    barForcesBarLabels.clear();
    barForcesHEdits.clear();
    barForcesVEdits.clear();

    // If no bars, clear everything except headers and return
    if (barCount == 0) {
        // Remove all rows except header (row 0)
        for (int row = barForcesGrid->rowCount() - 1; row >= 1; row--) {
            for (int col = 0; col < barForcesGrid->columnCount(); col++) {
                QLayoutItem *item = barForcesGrid->itemAtPosition(row, col);
                if (item && item->widget()) {
                    barForcesGrid->removeWidget(item->widget());
                    delete item->widget();
                }
            }
        }
        return;
    }

    // Remove all rows except header (row 0)
    for (int row = barForcesGrid->rowCount() - 1; row >= 1; row--) {
        for (int col = 0; col < barForcesGrid->columnCount(); col++) {
            QLayoutItem *item = barForcesGrid->itemAtPosition(row, col);
            if (item && item->widget()) {
                barForcesGrid->removeWidget(item->widget());
                delete item->widget();
            }
        }
    }

    // Add bar force inputs, starting from row 1
    for (int i = 0; i < barCount; i++) {
        int row = i + 1;

        // Bar label
        QLabel *barLabel = new QLabel(QString("Стержень %1").arg(i + 1));
        barForcesBarLabels.append(barLabel);

        // Force input fields with validators
        QLineEdit *hEdit = new QLineEdit();
        QLineEdit *vEdit = new QLineEdit();

        // Set up validators and placeholders
        QDoubleValidator *validator = new QDoubleValidator(-100000.0, 100000.0, 3, hEdit);
        hEdit->setValidator(validator);
        vEdit->setValidator(new QDoubleValidator(-100000.0, 100000.0, 3, vEdit));

        hEdit->setPlaceholderText("0.0");
        vEdit->setPlaceholderText("0.0");

        // Store references
        barForcesHEdits.append(hEdit);
        barForcesVEdits.append(vEdit);

        // Add to grid
        barForcesGrid->addWidget(barLabel, row, 0);
        barForcesGrid->addWidget(hEdit, row, 1);
        barForcesGrid->addWidget(vEdit, row, 2);
    }
}

void Sapr::updateSchemaData() {
    if (!schemaWidget)
        return;

    QVector<double> lengths;
    QVector<double> surfaces;

    for (int i = 0; i < barCount; i++) {
        lengths.append(getLengthValue(i));

        double surface = 1.0;
        if (i < surfaceEdits.size() && surfaceEdits[i]) {
            QString text = surfaceEdits[i]->text();
            if (!text.isEmpty()) {
                bool ok;
                surface = text.toDouble(&ok);
                if (!ok)
                    surface = 1.0;
            }
        }
        surfaces.append(surface);
    }

    schemaWidget->updateBars(lengths, surfaces, ui->checkBoxLeft->isChecked(),
                             ui->checkBoxRight->isChecked());
}

// Helper method to get node force values
QVector<double> Sapr::getNodeForces(int nodeIndex) {
    QVector<double> forces;

    if (nodeIndex >= 0 && nodeIndex < nodeForcesHEdits.size()) {
        double hForce = nodeForcesHEdits[nodeIndex]->text().toDouble();
        double vForce = nodeForcesVEdits[nodeIndex]->text().toDouble();
        forces << hForce << vForce;
    }

    return forces;
}

// Get all node forces as a list
QVector<QVector<double>> Sapr::getAllNodeForces() {
    QVector<QVector<double>> allForces;
    for (int i = 0; i < nodeForcesHEdits.size(); i++) {
        allForces.append(getNodeForces(i));
    }
    return allForces;
}

// Helper method to get bar force values
QVector<double> Sapr::getBarForces(int barIndex) {
    QVector<double> forces;

    if (barIndex >= 0 && barIndex < barForcesHEdits.size()) {
        double hForce = barForcesHEdits[barIndex]->text().toDouble();
        double vForce = barForcesVEdits[barIndex]->text().toDouble();
        forces << hForce << vForce;
    }

    return forces;
}

// Get all bar forces as a list
QVector<QVector<double>> Sapr::getAllBarForces() {
    QVector<QVector<double>> allForces;
    for (int i = 0; i < barForcesHEdits.size(); i++) {
        allForces.append(getBarForces(i));
    }
    return allForces;
}
