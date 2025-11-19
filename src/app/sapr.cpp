#include "sapr.h"
#include "filehandler.h"
#include "ui_sapr.h"
#include <QApplication>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>

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

void Sapr::setNodeForces(const QVector<double> &forces) { savedNodeForces = forces; }

void Sapr::setBarForces(const QVector<double> &forces) { savedBarForces = forces; }

Sapr::Sapr(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), barCount(0), headerNumber(nullptr),
      headerLength(nullptr), headerStartPoint(nullptr), headerAction(nullptr),
      nodeForcesGrid(nullptr), barForcesGrid(nullptr), schemaWidget(nullptr), calculator(nullptr),
      resultsTable(nullptr), stressTable(nullptr), calculationInProgress(false) {
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

    zoomInButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    zoomOutButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    fitButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    zoomInButton->setFixedWidth(40);
    zoomOutButton->setFixedWidth(40);
    fitButton->setMaximumWidth(80);

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
    QCheckBox *nodeForcesCheck = new QCheckBox("Сосредоточенные нагрузки");
    QCheckBox *barForcesCheck = new QCheckBox("Распределенные нагрузки");

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

    // Make the checkboxes' text expand with window and have a minimum width
    nodeNumbersCheck->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    barNumbersCheck->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    axisNumbersCheck->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    nodeForcesCheck->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    barForcesCheck->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    nodeNumbersCheck->setMinimumWidth(100);
    barNumbersCheck->setMinimumWidth(100);
    axisNumbersCheck->setMinimumWidth(100);
    nodeForcesCheck->setMinimumWidth(100);
    barForcesCheck->setMinimumWidth(100);

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

    setupResultsTables();
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
        headerTensileStrength = new QLabel("Допустимые напряжения (σ)");

        headerNumber->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        headerLength->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        headerStartPoint->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        headerSurface->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        headerElasticModulus->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        headerTensileStrength->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        headerAction->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

        // Set word wrap for labels with spaces
        headerLength->setWordWrap(true);
        headerSurface->setWordWrap(true);
        headerElasticModulus->setWordWrap(true);
        headerTensileStrength->setWordWrap(true);

        headerNumber->setMinimumWidth(30);
        headerLength->setMinimumWidth(30);
        headerStartPoint->setMinimumWidth(30);
        headerSurface->setMinimumWidth(30);
        headerElasticModulus->setMinimumWidth(30);
        headerTensileStrength->setMinimumWidth(30);
        headerAction->setMinimumWidth(30);

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

    numberLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    lengthEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    surfaceEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    elasticModulusEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    tensileStrengthEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    startPointLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    deleteButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    numberLabel->setMinimumHeight(30);
    lengthEdit->setMinimumHeight(30);
    surfaceEdit->setMinimumHeight(30);
    elasticModulusEdit->setMinimumHeight(30);
    tensileStrengthEdit->setMinimumHeight(30);
    startPointLabel->setMinimumHeight(30);
    deleteButton->setMinimumHeight(30);

    numberLabel->setMinimumWidth(50);
    lengthEdit->setMinimumWidth(50);
    surfaceEdit->setMinimumWidth(50);
    elasticModulusEdit->setMinimumWidth(50);
    tensileStrengthEdit->setMinimumWidth(50);
    startPointLabel->setMinimumWidth(50);
    deleteButton->setMinimumWidth(60);

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

        // If we get here, either shift was pressed or user confirmed
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
            nodeForcesEdits.clear();
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
            barForcesEdits.clear();
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
    if (index < savedNodeForces.size()) {
        savedNodeForces[index] = 0.0;
    }
    if (index + 1 < savedNodeForces.size()) {
        savedNodeForces[index + 1] = 0.0;
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
    QLabel *forceHeader = new QLabel("Fx");

    nodeHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    startOfHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    endOfHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    forceHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Add headers to grid
    nodeForcesGrid->addWidget(nodeHeader, 0, 0);
    nodeForcesGrid->addWidget(startOfHeader, 0, 1);
    nodeForcesGrid->addWidget(endOfHeader, 0, 2);
    nodeForcesGrid->addWidget(forceHeader, 0, 3);
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
    for (auto edit : nodeForcesEdits) {
        if (edit)
            delete edit;
    }

    nodeForcesNodeLabels.clear();
    nodeForcesStartLabels.clear();
    nodeForcesEndLabels.clear();
    nodeForcesEdits.clear();

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

        // Force input field with validator
        QLineEdit *forceEdit = new QLineEdit();

        nodeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        startLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        endLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        forceEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        // Set up validator and placeholder
        QDoubleValidator *validator = new QDoubleValidator(-100000.0, 100000.0, 3, forceEdit);
        forceEdit->setValidator(validator);
        forceEdit->setPlaceholderText("Н");

        // Connect the input changes to update the schema
        connect(forceEdit, &QLineEdit::textChanged, this, &Sapr::updateSchemaData);

        // Store references
        nodeForcesStartLabels.append(startLabel);
        nodeForcesEndLabels.append(endLabel);
        nodeForcesEdits.append(forceEdit);

        // Add to grid
        nodeForcesGrid->addWidget(nodeLabel, row, 0);
        nodeForcesGrid->addWidget(startLabel, row, 1);
        nodeForcesGrid->addWidget(endLabel, row, 2);
        nodeForcesGrid->addWidget(forceEdit, row, 3);
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
    QLabel *forceHeader = new QLabel("qx");

    barHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    forceHeader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Add headers to grid
    barForcesGrid->addWidget(barHeader, 0, 0);
    barForcesGrid->addWidget(forceHeader, 0, 1);
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
    for (auto edit : barForcesEdits) {
        if (edit)
            delete edit;
    }

    barForcesBarLabels.clear();
    barForcesEdits.clear();

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

        // Force input field with validator
        QLineEdit *forceEdit = new QLineEdit();

        barLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        forceEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        // Set up validator and placeholder
        QDoubleValidator *validator = new QDoubleValidator(-100000.0, 100000.0, 3, forceEdit);
        forceEdit->setValidator(validator);
        forceEdit->setPlaceholderText("Н/м");

        connect(forceEdit, &QLineEdit::textChanged, this, &Sapr::updateSchemaData);

        // Store references
        barForcesEdits.append(forceEdit);

        // Add to grid
        barForcesGrid->addWidget(barLabel, row, 0);
        barForcesGrid->addWidget(forceEdit, row, 1);
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

    QVector<double> nodeForces = getAllNodeForces();
    QVector<double> barForces = getAllBarForces();

    schemaWidget->updateNodeForces(nodeForces);
    schemaWidget->updateBarForces(barForces);

    schemaWidget->updateBars(lengths, surfaces, ui->checkBoxLeft->isChecked(),
                             ui->checkBoxRight->isChecked());
}

// Helper method to get node force values
double Sapr::getNodeForces(int nodeIndex) {
    if (nodeIndex >= 0 && nodeIndex < nodeForcesEdits.size()) {
        return nodeForcesEdits[nodeIndex]->text().toDouble();
    }
    return 0.0;
}

// Get all node forces as a list
QVector<double> Sapr::getAllNodeForces() {
    QVector<double> allForces;
    for (int i = 0; i < nodeForcesEdits.size(); i++) {
        allForces.append(getNodeForces(i));
    }
    return allForces;
}

// Helper method to get bar force values
double Sapr::getBarForces(int barIndex) {
    if (barIndex >= 0 && barIndex < barForcesEdits.size()) {
        return barForcesEdits[barIndex]->text().toDouble();
    }
    return 0.0;
}

// Get all bar forces as a list
QVector<double> Sapr::getAllBarForces() {
    QVector<double> allForces;
    for (int i = 0; i < barForcesEdits.size(); i++) {
        allForces.append(getBarForces(i));
    }
    return allForces;
}

void Sapr::saveNodeForces() {
    savedNodeForces.clear();
    for (int i = 0; i < nodeForcesEdits.size(); i++) {
        double force = 0.0;

        if (i < nodeForcesEdits.size() && nodeForcesEdits[i]) {
            QString text = nodeForcesEdits[i]->text();
            if (!text.isEmpty()) {
                force = text.toDouble();
            }
        }

        savedNodeForces.append(force);
    }
}

void Sapr::loadNodeForces() {
    for (int i = 0; i < nodeForcesEdits.size() && i < savedNodeForces.size(); i++) {
        if (nodeForcesEdits[i]) {
            if (savedNodeForces[i] != 0.0) {
                nodeForcesEdits[i]->setText(QString::number(savedNodeForces[i], 'f', 3));
            } else {
                nodeForcesEdits[i]->setText("");
            }
        }
    }
}

void Sapr::saveBarForces() {
    savedBarForces.clear();
    for (int i = 0; i < barForcesEdits.size(); i++) {
        double force = 0.0;

        if (i < barForcesEdits.size() && barForcesEdits[i]) {
            QString text = barForcesEdits[i]->text();
            if (!text.isEmpty()) {
                force = text.toDouble();
            }
        }

        savedBarForces.append(force);
    }
}

void Sapr::loadBarForces() {
    for (int i = 0; i < barForcesEdits.size() && i < savedBarForces.size(); i++) {
        if (barForcesEdits[i]) {
            if (savedBarForces[i] != 0.0) {
                barForcesEdits[i]->setText(QString::number(savedBarForces[i], 'f', 3));
            } else {
                barForcesEdits[i]->setText("");
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

void Sapr::setupResultsTables() {
    QVBoxLayout *tableLayout = new QVBoxLayout(ui->TableTab);

    // Таблица перемещений и усилий
    resultsTable = new QTableWidget();
    resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableLayout->addWidget(new QLabel("<h3>Результаты расчетов</h3>"));
    tableLayout->addWidget(resultsTable);

    // Таблица напряжений
    stressTable = new QTableWidget();
    stressTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableLayout->addWidget(new QLabel("<h3>Напряжения и проверка прочности</h3>"));
    tableLayout->addWidget(stressTable);

    // Кнопка для выполнения расчетов
    QPushButton *calculateButton = new QPushButton("Выполнить расчет");
    calculateButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; "
                                   "font-weight: bold; padding: 8px; }");
    connect(calculateButton, &QPushButton::clicked, this, &Sapr::performCalculations);
    tableLayout->addWidget(calculateButton);
    tableLayout->addStretch();
}

void Sapr::performCalculations() {

    // Защита от повторного входа
    if (calculationInProgress) {
        return;
    }

    calculationInProgress = true;

    // Временные переменные для результатов
    std::vector<double> displacements, forces, stresses;
    bool success = false;

    try {
        // Проверка базовых условий
        if (barCount == 0) {
            throw std::runtime_error("Нет стержней для расчета");
        }

        if (!ui->checkBoxLeft->isChecked() && !ui->checkBoxRight->isChecked()) {
            throw std::runtime_error("Система должна иметь хотя бы одну заделку");
        }

        // Проверяем данные стержней
        for (int i = 0; i < barCount; i++) {
            double L = getLengthValue(i);
            double A = getSurfaceValue(i);
            double E = getElasticModulusValue(i);

            if (L <= 0)
                throw std::runtime_error(QString("Стержень %1: длина должна быть положительной")
                                             .arg(i + 1)
                                             .toStdString());
            if (A <= 0)
                throw std::runtime_error(QString("Стержень %1: площадь должна быть положительной")
                                             .arg(i + 1)
                                             .toStdString());
            if (E <= 0)
                throw std::runtime_error(
                    QString("Стержень %1: модуль упругости должен быть положительным")
                        .arg(i + 1)
                        .toStdString());
        }

        // Удаляем старый калькулятор ПЕРЕД созданием нового
        if (calculator) {
            delete calculator;
            calculator = nullptr;
        }

        // Создаем новый калькулятор
        calculator = new RodSystemCalculator(barCount + 1);

        // Устанавливаем параметры стержней
        for (int i = 0; i < barCount; i++) {
            double L = getLengthValue(i);
            double A = getSurfaceValue(i);
            double E = getElasticModulusValue(i);
            double sigma_allow = getTensileStrengthValue(i);
            double q = getBarForces(i);

            calculator->setRod(i + 1, L, A, E, q, sigma_allow);
        }

        // Устанавливаем сосредоточенные силы
        QVector<double> nodeForces = getAllNodeForces();
        for (int i = 0; i < nodeForces.size(); i++) {
            calculator->setForce(i + 1, nodeForces[i]);
        }

        // Выполняем расчет
        calculator->calculate(displacements, forces, stresses, ui->checkBoxLeft->isChecked(),
                              ui->checkBoxRight->isChecked());

        success = true;

    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Ошибка расчета",
                              QString("Произошла ошибка при расчете: %1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "Ошибка расчета", "Неизвестная ошибка при расчете");
    }

    // Очищаем калькулятор независимо от результата
    if (calculator) {
        delete calculator;
        calculator = nullptr;
    }

    // Обновляем таблицы только если расчет успешен
    if (success && !displacements.empty()) {
        try {
            updateResultsTables(displacements, forces, stresses);
            QMessageBox::information(this, "Расчет завершен",
                                     "Расчеты успешно выполнены и отображены в таблицах.");
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Ошибка отображения",
                                  QString("Ошибка при обновлении таблиц: %1").arg(e.what()));
        }
    }

    calculationInProgress = false;
}

void Sapr::updateResultsTables(const std::vector<double> &displacements,
                               const std::vector<double> &forces,
                               const std::vector<double> &stresses) {

    // Проверка указателей таблиц
    if (!resultsTable || !stressTable) {
        return;
    }

    // Проверка размеров данных
    int nodeCount = barCount + 1;
    if (displacements.size() != static_cast<size_t>(nodeCount)) {
        return;
    }

    // Очищаем таблицы
    resultsTable->clear();
    stressTable->clear();

    // Таблица 1: Узловые перемещения
    resultsTable->setRowCount(nodeCount);
    resultsTable->setColumnCount(4);
    resultsTable->setHorizontalHeaderLabels(QStringList() << "Узел" << "Координата (м)"
                                                          << "Перемещение (м)" << "Статус");

    // Вычисляем координаты узлов
    QVector<double> nodeCoordinates(nodeCount, 0.0);
    for (int i = 1; i < nodeCount; i++) {
        nodeCoordinates[i] = nodeCoordinates[i - 1] + getLengthValue(i - 1);
    }

    // Заполняем таблицу перемещений
    for (int i = 0; i < nodeCount; i++) {
        // Убеждаемся, что у нас есть данные
        if (i >= displacements.size())
            break;

        resultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        resultsTable->setItem(i, 1,
                              new QTableWidgetItem(QString::number(nodeCoordinates[i], 'f', 3)));
        resultsTable->setItem(i, 2,
                              new QTableWidgetItem(QString::number(displacements[i], 'e', 6)));

        // Статус узла
        QTableWidgetItem *statusItem = new QTableWidgetItem();
        bool isLeftAnchor = (i == 0 && ui->checkBoxLeft->isChecked());
        bool isRightAnchor = (i == nodeCount - 1 && ui->checkBoxRight->isChecked());

        if (isLeftAnchor || isRightAnchor) {
            statusItem->setText("ЗАДЕЛКА");
            statusItem->setBackground(QBrush(QColor(200, 200, 200)));
        } else if (std::abs(displacements[i]) < 1e-10) {
            statusItem->setText("Неподвижен");
            statusItem->setBackground(QBrush(QColor(173, 216, 230)));
        } else {
            statusItem->setText("Свободен");
            statusItem->setBackground(QBrush(QColor(144, 238, 144)));
        }
        resultsTable->setItem(i, 3, statusItem);
    }

    // Таблица 2: Напряжения в узлах
    stressTable->setRowCount(nodeCount);
    stressTable->setColumnCount(4);
    stressTable->setHorizontalHeaderLabels(QStringList() << "Узел" << "Напряжение (Па)"
                                                         << "Допустимое (Па)" << "Статус");

    // Определяем допустимые напряжения для каждого узла
    QVector<double> nodeAllowedStresses(nodeCount, 200e6); // значение по умолчанию

    for (int i = 0; i < nodeCount; i++) {
        stressTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));

        if (i < stresses.size()) {
            double stress = std::abs(stresses[i]);
            double allowed = nodeAllowedStresses[i];

            stressTable->setItem(i, 1, new QTableWidgetItem(QString::number(stress, 'e', 6)));
            stressTable->setItem(i, 2, new QTableWidgetItem(QString::number(allowed, 'e', 6)));

            QTableWidgetItem *statusItem = new QTableWidgetItem();
            if (stress <= allowed) {
                statusItem->setText("НОРМА");
                statusItem->setBackground(QBrush(QColor(144, 238, 144)));
            } else {
                statusItem->setText("ПРЕВЫШЕНИЕ!");
                statusItem->setBackground(QBrush(QColor(255, 0, 0)));
                statusItem->setForeground(QBrush(QColor(255, 255, 255)));
            }
            stressTable->setItem(i, 3, statusItem);
        } else {
            // Заполняем пустыми значениями если данных нет
            stressTable->setItem(i, 1, new QTableWidgetItem("—"));
            stressTable->setItem(i, 2, new QTableWidgetItem("—"));
            stressTable->setItem(i, 3, new QTableWidgetItem("—"));
        }
    }

    resultsTable->resizeColumnsToContents();
    stressTable->resizeColumnsToContents();
}

double Sapr::getSurfaceValue(int index) {
    if (index < 0 || index >= surfaceEdits.size() || !surfaceEdits[index]) {
        return 0.0;
    }
    QString text = surfaceEdits[index]->text();
    if (text.isEmpty())
        return 0.0;

    bool ok;
    double value = text.toDouble(&ok);
    return ok ? value : 0.0;
}

double Sapr::getElasticModulusValue(int index) {
    if (index < 0 || index >= elasticModulusEdits.size() || !elasticModulusEdits[index]) {
        return 0.0;
    }
    QString text = elasticModulusEdits[index]->text();
    if (text.isEmpty())
        return 0.0;

    bool ok;
    double value = text.toDouble(&ok);
    return ok ? value : 0.0;
}

double Sapr::getTensileStrengthValue(int index) {
    if (index < 0 || index >= tensileStrengthEdits.size() || !tensileStrengthEdits[index]) {
        return 200e6; // значение по умолчанию
    }
    QString text = tensileStrengthEdits[index]->text();
    if (text.isEmpty())
        return 200e6;

    bool ok;
    double value = text.toDouble(&ok);
    return ok ? value : 200e6;
}
