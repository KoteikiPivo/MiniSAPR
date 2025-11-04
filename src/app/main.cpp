#include <QApplication>
#include <qapplication.h>
#include "sapr.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Sapr window;
    window.show();

    return app.exec();
}
