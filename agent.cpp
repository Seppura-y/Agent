#include "agent.h"
#include "capture_view.h"

#include <QScreen>

Agent::Agent(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    this->setGeometry(0, 0, 1024, 768);
    
    auto view = new CaptureView(this);
    //view->setGeometry(200, 100, 800, 600);

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();
    this->resize(width - 50, height - 100);



    view->setGeometry(0, 0, 1280, 720);
    view->show();
    view->setStyleSheet("background-color: rgb(55,55,55)");
}
