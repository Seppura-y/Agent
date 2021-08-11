#include "agent.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Agent w;
    w.show();
    return a.exec();
}
