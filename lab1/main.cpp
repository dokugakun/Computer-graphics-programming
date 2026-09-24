#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include "test.h"

int main(int argc, char *argv[])
{
    Tests::runAll();

    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
