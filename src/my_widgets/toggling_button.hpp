#ifndef QMUJOCOSIM_TOGGLING_BUTTON_HPP
#define QMUJOCOSIM_TOGGLING_BUTTON_HPP


#include <QPushButton>

class TogglingButton : public QPushButton {
public:
    explicit TogglingButton(QWidget *parent = nullptr) : QPushButton(parent) {
        setCheckable(true);
        setStyle();

        connect(this, &TogglingButton::clicked, [this]() {
            setStyle();
        });
    }

public slots:

    void setChecked(bool checked) {
        QPushButton::setChecked(checked);
        setStyle();
    };

private:
    void setStyle() {
        if (isChecked()) {
            setStyleSheet(
                    "QPushButton { background-color: #3498db; color: white; border-style: solid; border-width: 2px; border-radius: 5px; border-color: #2980b9; }");
        } else {
            setStyleSheet(
                    "QPushButton { background-color: #ecf0f1; color: #7f8c8d; border-style: solid; border-width: 2px; border-radius: 5px; border-color: #bdc3c7; }");
        }
    }
};

#endif //QMUJOCOSIM_TOGGLING_BUTTON_HPP
