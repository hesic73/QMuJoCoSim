#ifndef MUJOCO_SIMULATION_QT_RENDERING_SECTION_HPP
#define MUJOCO_SIMULATION_QT_RENDERING_SECTION_HPP

#include <QGridLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>

#include <QLabel>
#include <QPixmap>

#include "collapsible_section.h"


#include "my_widgets/toggling_button.hpp"

static QGroupBox *createSceneFlags() {
    auto group = new QGroupBox("OpenGL Effects");
    auto layout = new QGridLayout;


    auto shadow = new TogglingButton;
    shadow->setText("Shadow");

    auto wireframe = new TogglingButton;
    wireframe->setText("Wireframe");


    auto reflection = new TogglingButton;
    reflection->setText("Reflection");

    auto additive = new TogglingButton;
    additive->setText("Additive");

    auto skybox = new TogglingButton;
    skybox->setText("Skybox");

    auto fog = new TogglingButton;
    fog->setText("Fog");


    auto haze = new TogglingButton;
    haze->setText("Haze");

    auto segment = new TogglingButton;
    segment->setText("Segment");

    auto idColor = new TogglingButton;
    idColor->setText("Id Color");

    auto cullFace = new TogglingButton;
    cullFace->setText("Cull Face");


    layout->addWidget(shadow, 0, 0);
    layout->addWidget(wireframe, 0, 1);
    layout->addWidget(reflection, 1, 0);
    layout->addWidget(additive, 1, 1);
    layout->addWidget(skybox, 2, 0);
    layout->addWidget(fog, 2, 1);
    layout->addWidget(haze, 3, 0);
    layout->addWidget(segment, 3, 1);
    layout->addWidget(idColor, 4, 0);
    layout->addWidget(cullFace, 4, 1);

    group->setLayout(layout);
    return group;
}

class RenderingSection : public CollapsibleSection {
public:
    explicit RenderingSection(QWidget *parent = nullptr) : CollapsibleSection("Rendering", 300, parent) {
        auto myLayout = new QVBoxLayout;

        auto sceneFlags = createSceneFlags();

        myLayout->addWidget(sceneFlags);


        setContentLayout(myLayout);
    }

private:

};

#endif //MUJOCO_SIMULATION_QT_RENDERING_SECTION_HPP
