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

class RenderingEffects : public QGroupBox {
Q_OBJECT

public:
    explicit RenderingEffects(QWidget *parent = nullptr) : QGroupBox(parent) {
        auto layout = new QGridLayout;

        shadow = new TogglingButton;
        shadow->setText("Shadow");
        connect(shadow, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_SHADOW, shadow->isChecked());
        });

        // Wireframe button setup
        wireframe = new TogglingButton;
        wireframe->setText("Wireframe");
        connect(wireframe, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_WIREFRAME, wireframe->isChecked());
        });

        // Reflection button setup
        reflection = new TogglingButton;
        reflection->setText("Reflection");
        connect(reflection, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_REFLECTION, reflection->isChecked());
        });

        // Additive button setup
        additive = new TogglingButton;
        additive->setText("Additive");
        connect(additive, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_ADDITIVE, additive->isChecked());
        });

        // Skybox button setup
        skybox = new TogglingButton;
        skybox->setText("Skybox");
        connect(skybox, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_SKYBOX, skybox->isChecked());
        });

        // Fog button setup
        fog = new TogglingButton;
        fog->setText("Fog");
        connect(fog, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_FOG, fog->isChecked());
        });

        // Haze button setup
        haze = new TogglingButton;
        haze->setText("Haze");
        connect(haze, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_HAZE, haze->isChecked());
        });

        // Segment button setup
        segment = new TogglingButton;
        segment->setText("Segment");
        connect(segment, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_SEGMENT, segment->isChecked());
        });

        // Id Color button setup
        idColor = new TogglingButton;
        idColor->setText("Id Color");
        connect(idColor, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_IDCOLOR, idColor->isChecked());
        });

        // Cull Face button setup
        cullFace = new TogglingButton;
        cullFace->setText("Cull Face");
        connect(cullFace, &TogglingButton::clicked, [this]() {
            emit updateFlag(mjtRndFlag::mjRND_CULL_FACE, cullFace->isChecked());
        });

        // Layout configuration (no changes needed)
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

        setLayout(layout);
    }

public slots:

    /**
     * This method does not emit signals!!!
     */
    void setButtonChecked(mjtRndFlag flag, bool value) {
        switch (flag) {
            case mjRND_SHADOW:
                shadow->setChecked(value);
                break;
            case mjRND_WIREFRAME:
                wireframe->setChecked(value);
                break;
            case mjRND_REFLECTION:
                reflection->setChecked(value);
                break;
            case mjRND_ADDITIVE:
                additive->setChecked(value);
                break;
            case mjRND_SKYBOX:
                skybox->setChecked(value);
                break;
            case mjRND_FOG:
                fog->setChecked(value);
                break;
            case mjRND_HAZE:
                haze->setChecked(value);
                break;
            case mjRND_SEGMENT:
                segment->setChecked(value);
                break;
            case mjRND_IDCOLOR:
                idColor->setChecked(value);
                break;
            case mjRND_CULL_FACE:
                cullFace->setChecked(value);
                break;
            default:
                qCritical() << "Unknown flag!!!";
                break;
        }
    }

signals:

    void updateFlag(mjtRndFlag flag, bool value);

private:
    TogglingButton *shadow;
    TogglingButton *wireframe;
    TogglingButton *reflection;
    TogglingButton *additive;
    TogglingButton *skybox;
    TogglingButton *fog;
    TogglingButton *haze;
    TogglingButton *segment;
    TogglingButton *idColor;
    TogglingButton *cullFace;
};


class RenderingSection : public CollapsibleSection {
Q_OBJECT

public:
    explicit RenderingSection(QWidget *parent = nullptr) : CollapsibleSection("Rendering", 300, parent) {
        auto myLayout = new QVBoxLayout;

        sceneFlags = new RenderingEffects(this);

        myLayout->addWidget(sceneFlags);


        setContentLayout(myLayout);


        connect(sceneFlags, &RenderingEffects::updateFlag, [this](mjtRndFlag flag, bool value) {
            emit updateRenderingFlag(flag, value);
        });
    }

signals:

    void updateRenderingFlag(mjtRndFlag flag, bool value);

public slots:

    void setRenderingEffectsButtonChecked(mjtRndFlag flag, bool value) {
        sceneFlags->setButtonChecked(flag, value);
    };

private:
    RenderingEffects *sceneFlags;
};

#endif //QMUJOCOSIM_RENDERING_SECTION_HPP
