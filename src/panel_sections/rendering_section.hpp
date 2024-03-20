#ifndef MUJOCO_SIMULATION_QT_RENDERING_SECTION_HPP
#define MUJOCO_SIMULATION_QT_RENDERING_SECTION_HPP

#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPixmap>

#include "CollapsibleSection.h"

class RenderingSection : public CollapsibleSection {
public:
    explicit RenderingSection(QWidget *parent = nullptr) : CollapsibleSection("Rendering", 300, parent) {
        label = new QLabel(this);

        QPixmap dummyPixmap(150, 900);

        dummyPixmap.fill(Qt::red);

        label->setPixmap(dummyPixmap);

        auto myLayout = new QVBoxLayout;
        myLayout->addWidget(label);


        setContentLayout(myLayout);
    }

private:
    QLabel *label;
};

#endif //MUJOCO_SIMULATION_QT_RENDERING_SECTION_HPP
