#pragma once

#include <QtWidgets/QWidget>
#include "ui_agent.h"

class Agent : public QWidget
{
    Q_OBJECT

public:
    Agent(QWidget *parent = Q_NULLPTR);

private:
    Ui::AgentClass ui;
};
