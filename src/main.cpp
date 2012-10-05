#include <QtGui/QApplication>
#include "main/mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Resources);

    QApplication a(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    int res=a.exec();

    return res;
}
