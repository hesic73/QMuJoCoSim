#ifndef MUJOCO_SIMULATION_QT_CONTROL_PANEL_HPP
#define MUJOCO_SIMULATION_QT_CONTROL_PANEL_HPP

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Spoiler.h"

class ControlPanel : public QWidget {
public:
    explicit ControlPanel(QWidget *parent = nullptr) : QWidget(parent) {
        auto layout = new QVBoxLayout;
        layout->setAlignment(Qt::AlignTop);

        file = new Spoiler("File", 300, this);
        auto fileLayout = new QVBoxLayout;
        file->setContentLayout(fileLayout);


        simulation = new Spoiler("Simulation", 300, this);
        auto simulationLayout = new QVBoxLayout;
        simulation->setContentLayout(simulationLayout);


        layout->addWidget(file);
        layout->addWidget(simulation);

        setLayout(layout);

        setMaximumWidth(150);
    }


private:
    Spoiler *file;

    Spoiler *simulation;
};

#endif //MUJOCO_SIMULATION_QT_CONTROL_PANEL_HPP
