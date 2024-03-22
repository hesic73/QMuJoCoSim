#include <QPropertyAnimation>

#include "collapsible_section.h"

CollapsibleSection::CollapsibleSection(const QString &title, int animationDuration, QWidget *parent) : QWidget(parent),
                                                                                                       animationDuration(animationDuration) {
    toggleButton.setStyleSheet("QToolButton { border: none; font-size: 20px; }");
    toggleButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggleButton.setArrowType(Qt::ArrowType::RightArrow);
    toggleButton.setText(title);
    toggleButton.setCheckable(true);
    toggleButton.setChecked(false);
    toggleButton.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);


    contentArea.setStyleSheet("QScrollArea { background-color: white; border: none; }");
    contentArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // start out collapsed
    contentArea.setMaximumHeight(0);
    contentArea.setMinimumHeight(0);
    // let the entire widget grow and shrink with its content
    toggleAnimation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    toggleAnimation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    toggleAnimation.addAnimation(new QPropertyAnimation(&contentArea, "maximumHeight"));
    // don't waste space
    mainLayout.setSpacing(0);
    mainLayout.setContentsMargins(0, 0, 0, 0);
    mainLayout.addWidget(&toggleButton);
    mainLayout.addWidget(&contentArea);
    setLayout(&mainLayout);
    QObject::connect(&toggleButton, &QToolButton::clicked, [this](bool checked) {
        toggleButton.setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
        toggleAnimation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        toggleAnimation.start();
    });
}

void CollapsibleSection::setContentLayout(QLayout *contentLayout) {
    delete contentArea.layout();
    contentArea.setLayout(contentLayout);
    const auto collapsedHeight = sizeHint().height() - contentArea.maximumHeight();
    auto contentHeight = contentLayout->sizeHint().height();
    for (int i = 0; i < toggleAnimation.animationCount() - 1; ++i) {
        auto *spoilerAnimation = dynamic_cast<QPropertyAnimation *>(toggleAnimation.animationAt(i));
        spoilerAnimation->setDuration(animationDuration);
        spoilerAnimation->setStartValue(collapsedHeight);
        spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
    }
    auto *contentAnimation = dynamic_cast<QPropertyAnimation *>(toggleAnimation.animationAt(
            toggleAnimation.animationCount() - 1));
    contentAnimation->setDuration(animationDuration);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);
}