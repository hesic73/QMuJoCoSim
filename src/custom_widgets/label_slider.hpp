#ifndef QMUJOCOSIM_LABEL_SLIDER_HPP
#define QMUJOCOSIM_LABEL_SLIDER_HPP


#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <functional>

/**
 *
 */
class LabelSlider : public QLabel {
Q_OBJECT

public:
    explicit LabelSlider(QWidget *parent = nullptr,
                         const QColor &fillColor = Qt::darkRed,
                         const QColor &backgroundColor = Qt::lightGray,
                         std::function<QString(int)> valueDisplayFunction = [](int value) {
                             return QString::number(value);
                         })
            : QLabel(parent),
              m_fillColor(fillColor),
              m_backgroundColor(backgroundColor),
              m_valueDisplayFunction(valueDisplayFunction),
              m_minimum(0),
              m_maximum(100),
              m_value(0),
              m_invertedAppearance(false) {
        this->setMinimumHeight(50); // Ensure there's enough space for the number
        this->setStyleSheet(QString("QLabel { background-color : %1; }").arg(m_backgroundColor.name()));
        updateDisplay();
    }

    void setRange(int minimum, int maximum) {
        m_minimum = minimum;
        m_maximum = maximum;
        setValue(m_value); // Update the value to ensure it's within the new range
    }


    int value() const { return m_value; }

    void setInvertedAppearance(bool inverted) {
        m_invertedAppearance = inverted;
        updateDisplay();
    }

public slots:

    void setValue(int value) {
        // Clamp the value to the [minimum, maximum] range
        m_value = std::clamp(value, m_minimum, m_maximum);
        updateDisplay();
        emit valueChanged(m_value);
    }

    void setValueNoSignal(int value) {
        m_value = std::clamp(value, m_minimum, m_maximum);
        updateDisplay();
    }

signals:

    void valueChanged(int newValue);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        updateValueFromMouseEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons() & Qt::LeftButton) {
            updateValueFromMouseEvent(event);
        }
    }

    void paintEvent(QPaintEvent *event) override {
        QLabel::paintEvent(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRect rect = this->rect();
        float ratio = static_cast<float>(m_value - m_minimum) / (m_maximum - m_minimum);
        QRect filledRect = m_invertedAppearance ? QRect(rect.right() - rect.width() * ratio, rect.y(),
                                                        rect.width() * ratio, rect.height())
                                                : QRect(rect.x(), rect.y(), rect.width() * ratio, rect.height());

        painter.fillRect(filledRect, m_fillColor);

        painter.setPen(Qt::white);
        QString displayText = m_valueDisplayFunction(m_value);
        painter.drawText(rect, Qt::AlignCenter, displayText);
    }

private:
    QColor m_fillColor;
    QColor m_backgroundColor;
    std::function<QString(int)> m_valueDisplayFunction;
    int m_minimum;
    int m_maximum;
    int m_value;
    bool m_invertedAppearance;

    void updateValueFromMouseEvent(QMouseEvent *event) {
        auto x = qRound(event->position().x());
        int pos = m_invertedAppearance ? width() - x : x;
        int newValue = m_minimum + static_cast<int>((static_cast<double>(pos) / width()) * (m_maximum - m_minimum));
        setValue(newValue);
    }

    void updateDisplay() {
        update(); // Triggers a repaint to reflect the current value
    }
};


#endif //QMUJOCOSIM_LABEL_SLIDER_HPP
