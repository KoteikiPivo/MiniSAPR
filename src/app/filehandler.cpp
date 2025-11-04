#include "filehandler.h"
#include "sapr.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>

bool FileHandler::saveProject(Sapr *sapr, const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Ошибка", "Не удалось сохранить файл");
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Save header
    out << "# SAPR Project File\n";
    out << "# Generated on " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
        << "\n\n";

    // Save anchor states
    out << "[Anchors]\n";
    out << "Left=" << (sapr->getLeftAnchor() ? "true" : "false") << "\n";
    out << "Right=" << (sapr->getRightAnchor() ? "true" : "false") << "\n\n";

    // Save display settings
    out << "[Display]\n";
    if (sapr->schemaWidget) {
        out << "NodeNumbers=" << (sapr->schemaWidget->getShowNodeNumbers() ? "true" : "false")
            << "\n";
        out << "BarNumbers=" << (sapr->schemaWidget->getShowBarNumbers() ? "true" : "false")
            << "\n";
        out << "AxisNumbers=" << (sapr->schemaWidget->getShowAxisNumbers() ? "true" : "false")
            << "\n";
        out << "NodeForces=" << (sapr->schemaWidget->getShowNodeForces() ? "true" : "false")
            << "\n";
        out << "BarForces=" << (sapr->schemaWidget->getShowBarForces() ? "true" : "false") << "\n";
    }
    out << "\n";

    // Save bars
    out << "[Bars]\n";
    out << "Count=" << sapr->getBarCount() << "\n";
    for (int i = 0; i < sapr->getBarCount(); i++) {
        out << "Bar" << i + 1 << "=";
        out << sapr->getBarLength(i) << ",";
        out << sapr->getBarSurface(i) << ",";
        out << sapr->getBarElasticModulus(i) << ",";
        out << sapr->getBarTensileStrength(i) << "\n";
    }
    out << "\n";

    // Save node forces
    out << "[NodeForces]\n";
    QVector<QVector<double>> nodeForces = sapr->getAllNodeForces();
    out << "Count=" << nodeForces.size() << "\n";
    for (int i = 0; i < nodeForces.size(); i++) {
        out << "Node" << i + 1 << "=";
        if (nodeForces[i].size() >= 2) {
            out << nodeForces[i][0] << "," << nodeForces[i][1];
        } else {
            out << "0,0";
        }
        out << "\n";
    }
    out << "\n";

    // Save bar forces
    out << "[BarForces]\n";
    QVector<QVector<double>> barForces = sapr->getAllBarForces();
    out << "Count=" << barForces.size() << "\n";
    for (int i = 0; i < barForces.size(); i++) {
        out << "Bar" << i + 1 << "=";
        if (barForces[i].size() >= 2) {
            out << barForces[i][0] << "," << barForces[i][1];
        } else {
            out << "0,0";
        }
        out << "\n";
    }

    file.close();
    return true;
}

bool FileHandler::loadProject(Sapr *sapr, const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Ошибка", "Не удалось открыть файл");
        return false;
    }

    sapr->clearAllBars();

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QString section;
    QMap<QString, QString> values;

    // Vectors to store loaded data
    QVector<QVector<double>> loadedNodeForces;
    QVector<QVector<double>> loadedBarForces;
    bool anchorsLoaded = false;
    bool displayLoaded = false;
    bool barsLoaded = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        if (line.startsWith('[') && line.endsWith(']')) {
            // Process previous section
            processSection(section, values, sapr, loadedNodeForces, loadedBarForces, anchorsLoaded,
                           displayLoaded, barsLoaded);

            // Start new section
            section = line.mid(1, line.length() - 2);
            values.clear();
            continue;
        }

        // Key-value pair
        int equalsPos = line.indexOf('=');
        if (equalsPos != -1) {
            QString key = line.left(equalsPos).trimmed();
            QString value = line.mid(equalsPos + 1).trimmed();
            values[key] = value;
        }
    }

    // Process the last section
    processSection(section, values, sapr, loadedNodeForces, loadedBarForces, anchorsLoaded,
                   displayLoaded, barsLoaded);

    file.close();
    for (int i = 0; i < loadedNodeForces.size(); i++) {
        if (loadedNodeForces[i].size() >= 2) {
        }
    }
    for (int i = 0; i < loadedBarForces.size(); i++) {
        if (loadedBarForces[i].size() >= 2) {
        }
    }

    // Apply the loaded forces
    if (!loadedNodeForces.isEmpty()) {
        sapr->setNodeForces(loadedNodeForces);
    }
    if (!loadedBarForces.isEmpty()) {
        sapr->setBarForces(loadedBarForces);
    }

    // Apply forces to UI
    sapr->applyLoadedForces();

    return true;
}

// Add this helper method to process sections
void FileHandler::processSection(const QString &section, const QMap<QString, QString> &values,
                                 Sapr *sapr, QVector<QVector<double>> &loadedNodeForces,
                                 QVector<QVector<double>> &loadedBarForces, bool &anchorsLoaded,
                                 bool &displayLoaded, bool &barsLoaded) {
    if (section == "Anchors" && !anchorsLoaded) {
        if (values.contains("Left"))
            sapr->setLeftAnchor(values["Left"] == "true");
        if (values.contains("Right"))
            sapr->setRightAnchor(values["Right"] == "true");
        anchorsLoaded = true;
    } else if (section == "Display" && sapr->schemaWidget && !displayLoaded) {
        if (values.contains("NodeNumbers"))
            sapr->schemaWidget->setShowNodeNumbers(values["NodeNumbers"] == "true");
        if (values.contains("BarNumbers"))
            sapr->schemaWidget->setShowBarNumbers(values["BarNumbers"] == "true");
        if (values.contains("AxisNumbers"))
            sapr->schemaWidget->setShowAxisNumbers(values["AxisNumbers"] == "true");
        if (values.contains("NodeForces"))
            sapr->schemaWidget->setShowNodeForces(values["NodeForces"] == "true");
        if (values.contains("BarForces"))
            sapr->schemaWidget->setShowBarForces(values["BarForces"] == "true");
        displayLoaded = true;
    } else if (section == "Bars" && !barsLoaded) {
        if (values.contains("Count")) {
            int count = values["Count"].toInt();
            for (int i = 0; i < count; i++) {
                sapr->addBar();
            }
        }

        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            if (it.key().startsWith("Bar") && it.key() != "Count") {
                int barIndex = it.key().mid(3).toInt() - 1;
                QStringList barData = it.value().split(',');
                if (barIndex >= 0 && barData.size() >= 4) {
                    sapr->setBarProperties(barIndex, barData[0], barData[1], barData[2],
                                           barData[3]);
                }
            }
        }
        barsLoaded = true;
    } else if (section == "NodeForces") {
        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            if (it.key().startsWith("Node") && it.key() != "Count") {
                int nodeIndex = it.key().mid(4).toInt() - 1;
                QStringList forceData = it.value().split(',');
                if (nodeIndex >= 0 && forceData.size() >= 2) {
                    if (nodeIndex >= loadedNodeForces.size()) {
                        loadedNodeForces.resize(nodeIndex + 1);
                    }
                    QVector<double> forces;
                    double hForce = forceData[0].toDouble();
                    double vForce = forceData[1].toDouble();
                    forces << hForce << vForce;
                    loadedNodeForces[nodeIndex] = forces;
                }
            }
        }
    } else if (section == "BarForces") {
        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            if (it.key().startsWith("Bar") && it.key() != "Count") {
                int barIndex = it.key().mid(3).toInt() - 1;
                QStringList forceData = it.value().split(',');
                if (barIndex >= 0 && forceData.size() >= 2) {
                    if (barIndex >= loadedBarForces.size()) {
                        loadedBarForces.resize(barIndex + 1);
                    }
                    QVector<double> forces;
                    double hForce = forceData[0].toDouble();
                    double vForce = forceData[1].toDouble();
                    forces << hForce << vForce;
                    // DON'T add empty markers here - they will be added in setBarForces
                    loadedBarForces[barIndex] = forces;
                }
            }
        }
    }
}
