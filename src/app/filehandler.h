#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QString>
class Sapr;

class FileHandler {
  public:
    static bool saveProject(Sapr *sapr, const QString &fileName);
    static bool loadProject(Sapr *sapr, const QString &fileName);
    static void processSection(const QString &section, const QMap<QString, QString> &values,
                               Sapr *sapr, QVector<QVector<double>> &loadedNodeForces,
                               QVector<QVector<double>> &loadedBarForces, bool &anchorsLoaded,
                               bool &displayLoaded, bool &barsLoaded);
};

#endif // FILEHANDLER_H
