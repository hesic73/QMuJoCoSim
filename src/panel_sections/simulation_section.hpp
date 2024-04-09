#ifndef QMUJOCOSIM_SIMULATION_SECTION_HPP
#define QMUJOCOSIM_SIMULATION_SECTION_HPP


#include <functional>

#include <QGridLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>


#include <QSlider>
#include <QLabel>

#include "collapsible_section.h"

#include "my_widgets/label_slider.hpp"

class SimulationSection : public CollapsibleSection {
Q_OBJECT

public:
    explicit SimulationSection(QWidget *parent = nullptr) : CollapsibleSection("Simulation", 300, parent) {

        auto myLayout = new QVBoxLayout;
        myLayout->setContentsMargins(0, 0, 0, 0);

        historyLabel = new QLabel(this);
        historyLabel->setText("History");
        myLayout->addWidget(historyLabel);

        labelSlider = new LabelSlider(this, Qt::darkRed, Qt::lightGray, [](int value) {
            return QString::number(-value);
        });
        labelSlider->setInvertedAppearance(true);

        labelSlider->setEnabled(false);


        myLayout->addWidget(labelSlider);

        setContentLayout(myLayout);

    }

    void
    resetWhenModelIsNotNull(int simulationHistoryBufferSize, std::function<void(int)> onHistorySliderValueChanged) {
        labelSlider->setEnabled(true);
        labelSlider->setValue(0);
        labelSlider->setRange(0, simulationHistoryBufferSize - 1);


        connect(labelSlider, &LabelSlider::valueChanged, onHistorySliderValueChanged);
    }


    void resetWhenModelIsNull() {
        labelSlider->setEnabled(false);
        disconnect(labelSlider, &LabelSlider::valueChanged, nullptr, nullptr);
        labelSlider->setValue(0);
    }

public slots:

    void onKeyLeftPressed() {
        if (!labelSlider->isEnabled()) return;
        labelSlider->setValue(labelSlider->value() + 1);
    };

    void onKeyRightPressed() {
        if (!labelSlider->isEnabled()) return;
        labelSlider->setValue(labelSlider->value() - 1);
    }

private:
    QLabel *historyLabel;
    LabelSlider *labelSlider;
};

#endif //QMUJOCOSIM_SIMULATION_SECTION_HPP
