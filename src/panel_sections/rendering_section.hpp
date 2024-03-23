#ifndef QMUJOCOSIM_RENDERING_SECTION_HPP
#define QMUJOCOSIM_RENDERING_SECTION_HPP

#include <QGridLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>

#include <QLabel>
#include <QPixmap>

#include "collapsible_section.h"


#include "my_widgets/toggling_button.hpp"

class ModelElements : public QGroupBox {
Q_OBJECT

public:
    explicit ModelElements(QWidget *parent = nullptr) : QGroupBox("Model Elements", parent) {
        auto layout = new QGridLayout;
        layout->setContentsMargins(0, 0, 0, 0);

        for (int i = 0; i < mjtVisFlag::mjNVISFLAG; i++) {
            buttons[i] = new TogglingButton;
            buttons[i]->setText(mjVISSTRING[i][0]);

            connect(buttons[i], &TogglingButton::clicked, [this, i]() {
                emit updateFlag(static_cast<mjtVisFlag>(i), buttons[i]->isChecked());
            });


            layout->addWidget(buttons[i], i / 2, i % 2);
        }


        setLayout(layout);
    }

public slots:

    /**
     * This method does not emit signals!!!
     */
    void setButtonChecked(mjtVisFlag flag, bool value) {
        int i = static_cast<int>(flag);
        Q_ASSERT(i >= 0);
        Q_ASSERT(i < mjtVisFlag::mjNVISFLAG);
        buttons[i]->setChecked(value);
    }

signals:

    void updateFlag(mjtVisFlag flag, bool value);

private:
    TogglingButton *buttons[mjtVisFlag::mjNVISFLAG];
};


class RenderingEffects : public QGroupBox {
Q_OBJECT

public:
    explicit RenderingEffects(QWidget *parent = nullptr) : QGroupBox("Rendering Effects", parent) {
        auto layout = new QGridLayout;
        layout->setContentsMargins(0, 0, 0, 0);

        for (int i = 0; i < mjtRndFlag::mjNRNDFLAG; i++) {
            buttons[i] = new TogglingButton;
            buttons[i]->setText(mjRNDSTRING[i][0]);

            connect(buttons[i], &TogglingButton::clicked, [this, i]() {
                emit updateFlag(static_cast<mjtRndFlag>(i), buttons[i]->isChecked());
            });

            layout->addWidget(buttons[i], i / 2, i % 2);
        }

        setLayout(layout);
    }

public slots:

    void setButtonChecked(mjtRndFlag flag, bool value) {
        int i = static_cast<int>(flag);
        Q_ASSERT(i >= 0);
        Q_ASSERT(i < mjtRndFlag::mjNRNDFLAG);
        buttons[i]->setChecked(value);
    }

signals:

    void updateFlag(mjtRndFlag flag, bool value);

private:
    TogglingButton *buttons[mjtRndFlag::mjNRNDFLAG];
};


class RenderingSection : public CollapsibleSection {
Q_OBJECT

public:
    explicit RenderingSection(QWidget *parent = nullptr) : CollapsibleSection("Rendering", 300, parent) {
        auto myLayout = new QVBoxLayout;

        myLayout->setContentsMargins(0, 0, 0, 0);

        modelElements = new ModelElements(this);
        sceneFlags = new RenderingEffects(this);

        myLayout->addWidget(modelElements);
        myLayout->addWidget(sceneFlags);


        setContentLayout(myLayout);

        connect(modelElements, &ModelElements::updateFlag, [this](mjtVisFlag flag, bool value) {
            emit updateModelElementsFlag(flag, value);
        });

        connect(sceneFlags, &RenderingEffects::updateFlag, [this](mjtRndFlag flag, bool value) {
            emit updateRenderingFlag(flag, value);
        });
    }

signals:

    void updateModelElementsFlag(mjtVisFlag flag, bool value);

    void updateRenderingFlag(mjtRndFlag flag, bool value);

public slots:

    void setRenderingEffectsButtonChecked(mjtRndFlag flag, bool value) {
        sceneFlags->setButtonChecked(flag, value);
    };

    void setModelElementsButtonChecked(mjtVisFlag flag, bool value) {
        modelElements->setButtonChecked(flag, value);
    };

private:
    ModelElements *modelElements;
    RenderingEffects *sceneFlags;

};

#endif //QMUJOCOSIM_RENDERING_SECTION_HPP
