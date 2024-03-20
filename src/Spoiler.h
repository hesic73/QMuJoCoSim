//
// The class is copied from https://stackoverflow.com/a/37119983/15687189
//

#ifndef MUJOCO_SIMULATION_QT_SPOILER_H
#define MUJOCO_SIMULATION_QT_SPOILER_H


#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class Spoiler : public QWidget {
Q_OBJECT
private:
    QGridLayout mainLayout;
    QToolButton toggleButton;
    QFrame headerLine;
    QParallelAnimationGroup toggleAnimation;
    QScrollArea contentArea;
    int animationDuration{300};
public:
    explicit Spoiler(const QString & title = "", const int animationDuration = 300, QWidget *parent = nullptr);
    void setContentLayout(QLayout * contentLayout);
};

#endif //MUJOCO_SIMULATION_QT_SPOILER_H
