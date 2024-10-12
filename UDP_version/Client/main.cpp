#include "kernel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    kernel w;
    return a.exec();
}
