#ifndef QMUJOCOSIM_SIMULATION_SECTION_HPP
#define QMUJOCOSIM_SIMULATION_SECTION_HPP


#include <functional>

#include <QGridLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>


#include <QSlider>

#include "collapsible_section.h"

class SimulationSection : public CollapsibleSection {
Q_OBJECT

public:
    explicit SimulationSection(QWidget *parent = nullptr) : CollapsibleSection("Simulation", 300, parent) {

        auto myLayout = new QVBoxLayout;
        myLayout->setContentsMargins(0, 0, 0, 0);

        historySlider = new QSlider(this);
        historySlider->setOrientation(Qt::Orientation::Horizontal);
        historySlider->setInvertedAppearance(true);

        historySlider->setEnabled(false);


        myLayout->addWidget(historySlider);

        setContentLayout(myLayout);

    }

    void
    resetWhenModelIsNotNull(int simulationHistoryBufferSize, std::function<void(int)> onHistorySliderValueChanged) {
        historySlider->setEnabled(true);
        historySlider->setValue(0);
        historySlider->setRange(0, simulationHistoryBufferSize - 1);


        connect(historySlider, &QSlider::valueChanged, onHistorySliderValueChanged);
    }


    void resetWhenModelIsNull() {
        historySlider->setEnabled(false);
        disconnect(historySlider, &QSlider::valueChanged, nullptr, nullptr);
        historySlider->setValue(0);
    }


private:
    QSlider *historySlider;
};

#endif //QMUJOCOSIM_SIMULATION_SECTION_HPP
