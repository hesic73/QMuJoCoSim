#ifndef QMUJOCOSIM_SIMULATION_SECTION_HPP
#define QMUJOCOSIM_SIMULATION_SECTION_HPP


#include <functional>
#include <mutex>

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
    resetWhenModelIsNotNull(int simulationHistoryBufferSize, std::function<void(int)> onHistorySliderValueChanged,
                            std::function<void()> onValueIsZeroAndKeyRightPressed) {
        labelSlider->setEnabled(true);
        labelSlider->setValue(0);
        labelSlider->setRange(0, simulationHistoryBufferSize - 1);


        connect(labelSlider, &LabelSlider::valueChanged, onHistorySliderValueChanged);

        {
            std::unique_lock<std::mutex> lock(mtx);
            _onValueIsZeroAndKeyRightPressed = onValueIsZeroAndKeyRightPressed;
        }

    }


    void resetWhenModelIsNull() {
        labelSlider->setEnabled(false);
        disconnect(labelSlider, &LabelSlider::valueChanged, nullptr, nullptr);
        labelSlider->setValue(0);


        {
            std::unique_lock<std::mutex> lock(mtx);
            _onValueIsZeroAndKeyRightPressed = nullptr;
        }
    }


public slots:


    void setSliderValueNoSignal(int value) {
        labelSlider->setValueNoSignal(value);
    };

    // Step backward.
    void onKeyLeftPressed() {
        if (!labelSlider->isEnabled()) return;
        labelSlider->setValue(labelSlider->value() + 1);
    };


    // Step forward. This is a bit tricky, as it may calculate the new state and add it to the history buffer.
    void onKeyRightPressed() {
        if (!labelSlider->isEnabled()) return;
        if (labelSlider->value() != 0) {
            labelSlider->setValue(labelSlider->value() - 1);
        } else {
            {
                std::unique_lock<std::mutex> lock(mtx);
                if (_onValueIsZeroAndKeyRightPressed) {
                    _onValueIsZeroAndKeyRightPressed();
                }
            }
        }

    }

private:
    QLabel *historyLabel;
    LabelSlider *labelSlider;

    std::mutex mtx;
    std::function<void()> _onValueIsZeroAndKeyRightPressed = nullptr;
};

#endif //QMUJOCOSIM_SIMULATION_SECTION_HPP
