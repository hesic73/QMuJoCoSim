#ifndef QMUJOCOSIM_CONTROL_PANEL_HPP
#define QMUJOCOSIM_CONTROL_PANEL_HPP

#include <QWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "panel_sections/rendering_section.hpp"

class ControlPanel : public QWidget {
public:
    explicit ControlPanel(QWidget *parent = nullptr) : QWidget(parent) {
        auto layout = new QVBoxLayout;
        layout->setAlignment(Qt::AlignTop);

        renderingSection = new RenderingSection(this);
        layout->addWidget(renderingSection);

        setLayout(layout);
    }


public:
    RenderingSection *renderingSection;

};

#endif //QMUJOCOSIM_CONTROL_PANEL_HPP
