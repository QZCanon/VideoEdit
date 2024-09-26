#include "mainwindow.h"

#include <QApplication>

#if defined(Q_OS_WIN)
#undef main
#endif
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
