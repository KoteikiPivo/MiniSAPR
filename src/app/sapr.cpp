#include "sapr.h"
#include "ui_sapr.h"
#include "filehandler.h"
#include <QLabel>
#include <QPushButton>
#include <QDoubleValidator>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

// Public getters for FileHandler
bool Sapr::getLeftAnchor() const { return ui->checkBoxLeft->isChecked(); }
bool Sapr::getRightAnchor() const { return ui->checkBoxRight->isChecked(); }

QString Sapr::getBarLength(int index) const {
    return (index >= 0 && index < lengthEdits.size()) ? lengthEdits[index]->text() : "";
}

QString Sapr::getBarSurface(int index) const {
    return (index >= 0 && index < surfaceEdits.size()) ? surfaceEdits[index]->text() : "";
}

QString Sapr::getBarElasticModulus(int index) const {
    return (index >= 0 && index < elasticModulusEdits.size()) ? elasticModulusEdits[index]->text()
                                                              : "";
}

QString Sapr::getBarTensileStrength(int index) const {
    return (index >= 0 && index < tensileStrengthEdits.size()) ? tensileStrengthEdits[index]->text()
                                                               : "";
}

// Public setters for FileHandler
void Sapr::setLeftAnchor(bool anchored) { ui->checkBoxLeft->setChecked(anchored); }
void Sapr::setRightAnchor(bool anchored) { ui->checkBoxRight->setChecked(anchored); }

void Sapr::clearAllBars() {
    while (barCount > 0) {
        removeBar(0);
    }
}

void Sapr::addBar() { on_BarsAdd_clicked(); }

void Sapr::setBarProperties(int index, const QString &length, const QString &surface,
                            const QString &elasticModulus, const QString &tensileStrength) {
    if (index >= 0 && index < barCount) {
        if (index < lengthEdits.size())
            lengthEdits[index]->setText(length);
        if (index < surfaceEdits.size())
            surfaceEdits[index]->setText(surface);
        if (index < elasticModulusEdits.size())
            elasticModulusEdits[index]->setText(elasticModulus);
        if (index < tensileStrengthEdits.size())
            tensileStrengthEdits[index]->setText(tensileStrength);
    }
}

void Sapr::setNodeForces(const QVector<QVector<double>> &forces) {
    savedNodeForces.clear();
    for (const auto &force : forces) {
        if (force.size() >= 2) {
            QVector<double> formattedForce;
            double hForce = force[0];
            double vForce = force[1];
            formattedForce << hForce << vForce;
            // For file loading: always assume values are present (not empty)
            formattedForce << 0.0 << 0.0; // Mark both as non-empty
            savedNodeForces.append(formattedForce);
        }
    }
}

void Sapr::setBarForces(const QVector<QVector<double>> &forces) {
    savedBarForces.clear();
    for (const auto &force : forces) {
        if (force.size() >= 2) {
            QVector<double> formattedForce;
            double hForce = force[0];
            double vForce = force[1];
            formattedForce << hForce << vForce;
            // For file loading: always assume values are present (not empty)
            formattedForce << 0.0 << 0.0; // Mark both as non-empty
            savedBarForces.append(formattedForce);
        }
    }
}

Sapr::Sapr(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), barCount(0), headerNumber(nullptr),
      headerLength(nullptr), headerStartPoint(nullptr), headerAction(nullptr),
      nodeForcesGrid(nullptr), barForcesGrid(nullptr), schemaWidget(nullptr) {
    ui->setupUi(this);

    ui->BarsGrid->setVerticalSpacing(2);

    QWidget *schemaContainer = new QWidget(ui->SchemaTab);
    QVBoxLayout *schemaContainerLayout = new QVBoxLayout(schemaContainer);
    schemaContainerLayout->setContentsMargins(0, 0, 0, 0);
    schemaContainerLayout->setSpacing(0);

    schemaWidget = new SchemaWidget(schemaContainer);
    schemaContainerLayout->addWidget(schemaWidget);

    QVBoxLayout *schemaLayout = new QVBoxLayout(ui->SchemaTab);
    schemaLayout->setContentsMargins(0, 0, 0, 0);
    schemaLayout->setSpacing(0);

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
    connect(ui->ForcesTabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (schemaWidget) {
            updateSchemaData();
        }
    });

    QHBoxLayout *controlLayout = new QHBoxLayout();
    QPushButton *zoomInButton = new QPushButton("+");
    QPushButton *zoomOutButton = new QPushButton("-");
    QPushButton *fitButton = new QPushButton("Уместить");

    connect(zoomInButton, &QPushButton::clicked, this, [this]() {
        if (schemaWidget) {
            double currentScale = schemaWidget->getScale();
            schemaWidget->setScale(currentScale * 1.2);
        }
    });
    connect(zoomOutButton, &QPushButton::clicked, this, [this]() {
        if (schemaWidget) {
            double currentScale = schemaWidget->getScale();
            schemaWidget->setScale(currentScale / 1.2);
        }
    });
    connect(fitButton, &QPushButton::clicked, this, [this]() {
        if (schemaWidget) {
            schemaWidget->fitToView();
        }
    });

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);

    QCheckBox *nodeNumbersCheck = new QCheckBox("Номера узлов");
    QCheckBox *barNumbersCheck = new QCheckBox("Номера стержней");
    QCheckBox *axisNumbersCheck = new QCheckBox("Координаты");
    QCheckBox *nodeForcesCheck = new QCheckBox("Точечные силы");
    QCheckBox *barForcesCheck = new QCheckBox("Продольные силы");
    nodeNumbersCheck->setChecked(true);
    barNumbersCheck->setChecked(true);
    axisNumbersCheck->setChecked(true);
    nodeForcesCheck->setChecked(true);
    barForcesCheck->setChecked(true);

    connect(nodeNumbersCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (schemaWidget) {
            schemaWidget->setShowNodeNumbers(checked);
        }
    });
    connect(barNumbersCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (schemaWidget) {
            schemaWidget->setShowBarNumbers(checked);
        }
    });
    connect(axisNumbersCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (schemaWidget) {
            schemaWidget->setShowAxisNumbers(checked);
        }
    });
    connect(nodeForcesCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (schemaWidget) {
            schemaWidget->setShowNodeForces(checked);
        }
    });
    connect(barForcesCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (schemaWidget) {
            schemaWidget->setShowBarForces(checked);
        }
    });

    controlLayout->addWidget(zoomInButton);
    controlLayout->addWidget(zoomOutButton);
    controlLayout->addWidget(fitButton);
    controlLayout->addWidget(separator);
    controlLayout->addWidget(nodeNumbersCheck);
    controlLayout->addWidget(barNumbersCheck);
    controlLayout->addWidget(axisNumbersCheck);
    controlLayout->addWidget(nodeForcesCheck);
    controlLayout->addWidget(barForcesCheck);
    controlLayout->addStretch();

    schemaLayout->addLayout(controlLayout);
    schemaLayout->addWidget(schemaContainer);
}

bool firstAdd = true;
void Sapr::on_BarsAdd_clicked() {
    saveNodeForces();
    saveBarForces();

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
    lengthEdit->setPlaceholderText("м");

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

    updateNodeForces(false);
    updateBarForces(false);
    loadNodeForces();
    loadBarForces();

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

        // Check if shift key is pressed for immediate deletion
        bool shiftPressed = QApplication::keyboardModifiers() & Qt::ShiftModifier;

        if (!shiftPressed) {
            // Show confirmation dialog
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(
                this, "Подтверждение удаления",
                QString("Вы действительно хотите удалить стержень %1?").arg(index + 1),
                QMessageBox::Yes | QMessageBox::No);

            if (reply != QMessageBox::Yes) {
                return; // User canceled deletion
            }
        }

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

    saveNodeForces();
    saveBarForces();

    // Delete the bar widgets
    delete numberLabels[index];
    delete startPointLabels[index];
    delete lengthEdits[index];
    delete surfaceEdits[index];
    delete elasticModulusEdits[index];
    delete tensileStrengthEdits[index];
    delete deleteButtons[index];

    // Remove from vectors
    numberLabels.remove(index);
    startPointLabels.remove(index);
    lengthEdits.remove(index);
    surfaceEdits.remove(index);
    elasticModulusEdits.remove(index);
    tensileStrengthEdits.remove(index);
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

        // Clear saved forces
        savedNodeForces.clear();
        savedBarForces.clear();

        // Clear the grid completely
        QLayoutItem *child;
        while ((child = ui->BarsGrid->takeAt(0)) != nullptr) {
            if (child->widget()) {
                ui->BarsGrid->removeWidget(child->widget());
                delete child->widget();
            }
            delete child;
        }
        // Clear node forces headers and widgets
        if (nodeForcesGrid) {
            QLayoutItem *forceChild;
            while ((forceChild = nodeForcesGrid->takeAt(0)) != nullptr) {
                if (forceChild->widget()) {
                    nodeForcesGrid->removeWidget(forceChild->widget());
                    delete forceChild->widget();
                }
                delete forceChild;
            }

            // Clear the vectors
            nodeForcesNodeLabels.clear();
            nodeForcesStartLabels.clear();
            nodeForcesEndLabels.clear();
            nodeForcesHEdits.clear();
            nodeForcesVEdits.clear();
        }

        // Clear bar forces headers and widgets
        if (barForcesGrid) {
            QLayoutItem *barForceChild;
            while ((barForceChild = barForcesGrid->takeAt(0)) != nullptr) {
                if (barForceChild->widget()) {
                    barForcesGrid->removeWidget(barForceChild->widget());
                    delete barForceChild->widget();
                }
                delete barForceChild;
            }

            // Clear the vectors
            barForcesBarLabels.clear();
            barForcesHEdits.clear();
            barForcesVEdits.clear();
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
            ui->BarsGrid->addWidget(deleteButtons[i], row, 6);

            ui->BarsGrid->setRowStretch(row, 0);
        }
    }

    // Clear the forces at the nodes of the deleted bar
    // When removing bar at index, we clear nodes at index and index+1
    if (index < savedNodeForces.size()) {
        // Clear the node at the start of the deleted bar
        savedNodeForces[index] = QVector<double>()
                                 << 0.0 << 0.0 << 1.0 << 1.0; // Mark both as empty
    }
    if (index + 1 < savedNodeForces.size()) {
        // Clear the node at the end of the deleted bar
        savedNodeForces[index + 1] = QVector<double>()
                                     << 0.0 << 0.0 << 1.0 << 1.0; // Mark both as empty
    }

    // For bar forces, remove the bar force at the deleted index
    if (index < savedBarForces.size()) {
        savedBarForces.remove(index);
    }

    updateNodeForces(false);
    updateBarForces(false);
    loadNodeForces();
    loadBarForces();

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
    nodeForcesGrid->addWidget(endOfHeader, 0, 1);
    nodeForcesGrid->addWidget(startOfHeader, 0, 2);
    nodeForcesGrid->addWidget(horizontalHeader, 0, 3);
    nodeForcesGrid->addWidget(verticalHeader, 0, 4);
}
void Sapr::updateNodeForces(bool skipSave) {
    // Safety check
    if (!nodeForcesGrid)
        return;

    if (!skipSave)
        saveNodeForces();

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

        nodeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        startLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        endLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        hEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        vEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        // Set up validators and placeholders
        QDoubleValidator *validator = new QDoubleValidator(-100000.0, 100000.0, 3, hEdit);
        hEdit->setValidator(validator);
        vEdit->setValidator(new QDoubleValidator(-100000.0, 100000.0, 3, vEdit));

        hEdit->setPlaceholderText("Н");
        vEdit->setPlaceholderText("Н");

        // Connect the input changes to update the schema
        connect(hEdit, &QLineEdit::textChanged, this, &Sapr::updateSchemaData);
        connect(vEdit, &QLineEdit::textChanged, this, &Sapr::updateSchemaData);

        // Store references
        nodeForcesStartLabels.append(startLabel);
        nodeForcesEndLabels.append(endLabel);
        nodeForcesHEdits.append(hEdit);
        nodeForcesVEdits.append(vEdit);

        // Add to grid
        nodeForcesGrid->addWidget(nodeLabel, row, 0);
        nodeForcesGrid->addWidget(endLabel, row, 1);
        nodeForcesGrid->addWidget(startLabel, row, 2);
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

void Sapr::updateBarForces(bool skipSave) {
    // Safety check
    if (!barForcesGrid)
        return;

    if (!skipSave)
        saveBarForces();

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
        QLabel *barLabel = new QLabel(QString("%1").arg(i + 1));
        barForcesBarLabels.append(barLabel);

        // Force input fields with validators
        QLineEdit *hEdit = new QLineEdit();
        QLineEdit *vEdit = new QLineEdit();

        barLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        hEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        vEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        // Set up validators and placeholders
        QDoubleValidator *validator = new QDoubleValidator(-100000.0, 100000.0, 3, hEdit);
        hEdit->setValidator(validator);
        vEdit->setValidator(new QDoubleValidator(-100000.0, 100000.0, 3, vEdit));

        hEdit->setPlaceholderText("Н/м");
        vEdit->setPlaceholderText("Н/м");

        connect(hEdit, &QLineEdit::textChanged, this, &Sapr::updateSchemaData);
        connect(vEdit, &QLineEdit::textChanged, this, &Sapr::updateSchemaData);

        // Store references
        barForcesHEdits.append(hEdit);
        barForcesVEdits.append(vEdit);

        // Add to grid
        barForcesGrid->addWidget(barLabel, row, 0);
        barForcesGrid->addWidget(hEdit, row, 1);
        barForcesGrid->addWidget(vEdit, row, 2);
    }

    loadBarForces();
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

    QVector<QVector<double>> nodeForces = getAllNodeForces();
    QVector<QVector<double>> barForces = getAllBarForces();

    schemaWidget->updateNodeForces(nodeForces);
    schemaWidget->updateBarForces(barForces);

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

void Sapr::saveNodeForces() {
    savedNodeForces.clear();
    for (int i = 0; i < nodeForcesHEdits.size(); i++) {
        QVector<double> forces;
        if (i < nodeForcesHEdits.size() && nodeForcesHEdits[i]) {
            QString hText = nodeForcesHEdits[i]->text();
            if (!hText.isEmpty()) {
                forces << hText.toDouble();
            } else {
                forces << 0.0; // Use a special marker for empty values
            }
        } else {
            forces << 0.0;
        }

        if (i < nodeForcesVEdits.size() && nodeForcesVEdits[i]) {
            QString vText = nodeForcesVEdits[i]->text();
            if (!vText.isEmpty()) {
                forces << vText.toDouble();
            } else {
                forces << 0.0; // Use a special marker for empty values
            }
        } else {
            forces << 0.0;
        }

        // Add a flag to indicate which values were originally empty
        bool hWasEmpty = (i < nodeForcesHEdits.size() && nodeForcesHEdits[i] &&
                          nodeForcesHEdits[i]->text().isEmpty());
        bool vWasEmpty = (i < nodeForcesVEdits.size() && nodeForcesVEdits[i] &&
                          nodeForcesVEdits[i]->text().isEmpty());
        forces << (hWasEmpty ? 1.0 : 0.0); // Use 1.0 to mark empty horizontal force
        forces << (vWasEmpty ? 1.0 : 0.0); // Use 1.0 to mark empty vertical force

        savedNodeForces.append(forces);
    }
}
/*
void Sapr::loadNodeForces() {
    for (int i = 0; i < nodeForcesHEdits.size() && i < savedNodeForces.size(); i++) {
        if (nodeForcesHEdits[i] && savedNodeForces[i].size() >= 3) {
            if (savedNodeForces[i][2] == 1.0) { // Was originally empty
                nodeForcesHEdits[i]->setText("");
            } else {
                nodeForcesHEdits[i]->setText(QString::number(savedNodeForces[i][0], 'f', 3));
            }
        }
        if (nodeForcesVEdits[i] && savedNodeForces[i].size() >= 4) {
            if (savedNodeForces[i][3] == 1.0) { // Was originally empty
                nodeForcesVEdits[i]->setText("");
            } else {
                nodeForcesVEdits[i]->setText(QString::number(savedNodeForces[i][1], 'f', 3));
            }
        }
    }
}
*/

void Sapr::saveBarForces() {
    savedBarForces.clear();
    for (int i = 0; i < barForcesHEdits.size(); i++) {
        QVector<double> forces;
        if (i < barForcesHEdits.size() && barForcesHEdits[i]) {
            QString hText = barForcesHEdits[i]->text();
            if (!hText.isEmpty()) {
                forces << hText.toDouble();
            } else {
                forces << 0.0;
            }
        } else {
            forces << 0.0;
        }

        if (i < barForcesVEdits.size() && barForcesVEdits[i]) {
            QString vText = barForcesVEdits[i]->text();
            if (!vText.isEmpty()) {
                forces << vText.toDouble();
            } else {
                forces << 0.0;
            }
        } else {
            forces << 0.0;
        }

        // Add flags for empty values
        bool hWasEmpty = (i < barForcesHEdits.size() && barForcesHEdits[i] &&
                          barForcesHEdits[i]->text().isEmpty());
        bool vWasEmpty = (i < barForcesVEdits.size() && barForcesVEdits[i] &&
                          barForcesVEdits[i]->text().isEmpty());
        forces << (hWasEmpty ? 1.0 : 0.0);
        forces << (vWasEmpty ? 1.0 : 0.0);

        savedBarForces.append(forces);
    }
}

/*
void Sapr::loadBarForces() {
    for (int i = 0; i < barForcesHEdits.size() && i < savedBarForces.size(); i++) {
        if (barForcesHEdits[i] && savedBarForces[i].size() >= 3) {
            if (savedBarForces[i][2] == 1.0) {
                barForcesHEdits[i]->setText("");
            } else {
                barForcesHEdits[i]->setText(QString::number(savedBarForces[i][0], 'f', 3));
            }
        }
        if (barForcesVEdits[i] && savedBarForces[i].size() >= 4) {
            if (savedBarForces[i][3] == 1.0) {
                barForcesVEdits[i]->setText("");
            } else {
                barForcesVEdits[i]->setText(QString::number(savedBarForces[i][1], 'f', 3));
            }
        }
    }
}
*/

void Sapr::loadNodeForces() {
    for (int i = 0; i < nodeForcesHEdits.size() && i < savedNodeForces.size(); i++) {
        if (nodeForcesHEdits[i] && savedNodeForces[i].size() >= 4) {
            // Only set text if the empty marker is 0.0 (not empty)
            if (savedNodeForces[i][2] == 0.0) {
                QString hValue = QString::number(savedNodeForces[i][0], 'f', 3);
                nodeForcesHEdits[i]->setText(hValue);
            } else {
                nodeForcesHEdits[i]->setText("");
            }
        }
        if (nodeForcesVEdits[i] && savedNodeForces[i].size() >= 4) {
            // Only set text if the empty marker is 0.0 (not empty)
            if (savedNodeForces[i][3] == 0.0) {
                QString vValue = QString::number(savedNodeForces[i][1], 'f', 3);
                nodeForcesVEdits[i]->setText(vValue);
            } else {
                nodeForcesVEdits[i]->setText("");
            }
        }
    }
}

void Sapr::loadBarForces() {
    for (int i = 0; i < barForcesHEdits.size() && i < savedBarForces.size(); i++) {
        if (barForcesHEdits[i] && savedBarForces[i].size() >= 4) {
            // Only set text if the empty marker is 0.0 (not empty)
            if (savedBarForces[i][2] == 0.0) {
                QString hValue = QString::number(savedBarForces[i][0], 'f', 3);
                barForcesHEdits[i]->setText(hValue);
            } else {
                barForcesHEdits[i]->setText("");
            }
        }
        if (barForcesVEdits[i] && savedBarForces[i].size() >= 4) {
            // Only set text if the empty marker is 0.0 (not empty)
            if (savedBarForces[i][3] == 0.0) {
                QString vValue = QString::number(savedBarForces[i][1], 'f', 3);
                barForcesVEdits[i]->setText(vValue);
            } else {
                barForcesVEdits[i]->setText("");
            }
        }
    }
}

void Sapr::on_action_2_triggered() {
    QString fileName =
        QFileDialog::getSaveFileName(this, "Сохранить проект", "", "SAPR Files (*.sapr)");
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".sapr", Qt::CaseInsensitive)) {
            fileName += ".sapr";
        }
        if (FileHandler::saveProject(this, fileName)) {
            QMessageBox::information(this, "Успех", "Проект сохранен");
        }
    }
}

void Sapr::on_action_3_triggered() {
    QString fileName =
        QFileDialog::getOpenFileName(this, "Открыть проект", "", "SAPR Files (*.sapr)");
    if (!fileName.isEmpty()) {
        if (FileHandler::loadProject(this, fileName)) {
            // Update forces with loaded data
            updateNodeForces(false);
            updateBarForces(false);
            loadNodeForces();
            loadBarForces();

            // Update schema
            updateStartPoints();
            updateSchemaData();

            QMessageBox::information(this, "Успех", "Проект загружен");
        }
    }
}

void Sapr::on_action_5_triggered() { QApplication::quit(); }

void Sapr::applyLoadedForces() {
    // Update forces with loaded data
    updateNodeForces(true);
    updateBarForces(true);
    loadNodeForces();
    loadBarForces();

    // Update schema
    updateStartPoints();
    updateSchemaData();
}
