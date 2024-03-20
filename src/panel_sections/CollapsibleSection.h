//
// The class is adapted from https://stackoverflow.com/a/37119983/15687189
//

#ifndef MUJOCO_SIMULATION_QT_COLLAPSIBLESECTION_H
#define MUJOCO_SIMULATION_QT_COLLAPSIBLESECTION_H


#include <QVBoxLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QtWidgets/QWidget>

class CollapsibleSection : public QWidget {
Q_OBJECT

private:
    QVBoxLayout mainLayout;
    QToolButton toggleButton;
    QParallelAnimationGroup toggleAnimation;
    QScrollArea contentArea;
    const int animationDuration{300};
public:
    explicit CollapsibleSection(const QString &title = "", int animationDuration = 300, QWidget *parent = nullptr);

protected:
    void setContentLayout(QLayout *contentLayout);

private:
    void setLayout(QLayout *layout) {
        QWidget::setLayout(layout);
    }
};

#endif //MUJOCO_SIMULATION_QT_COLLAPSIBLESECTION_H
