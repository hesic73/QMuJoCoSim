#ifndef QMUJOCOSIM_MY_WINDOW_CONTAINER_HPP
#define QMUJOCOSIM_MY_WINDOW_CONTAINER_HPP

#include <QWidget>
#include "mujoco_opengl_window.hpp"
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QList>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>


class MyWindowContainer : public QWidget {
Q_OBJECT

public:
    explicit MyWindowContainer(MuJoCoOpenGLWindow *window, QWidget *parent = nullptr) : QWidget(parent) {
        setAcceptDrops(true);
        this->window = window;
        auto container = QWidget::createWindowContainer(window, this);
        auto layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);  // Set the layout margins to zero
        layout->setSpacing(0);                   // Set the spacing to zero if necessary
        layout->addWidget(container, 1);         // Add the container with a stretch factor

        // Install event filter on the QWindow
        window->installEventFilter(this);
    }

signals:

    void keyLeftPressed();

    void keyRightPressed();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasUrls()) {
            QList<QUrl> urls = event->mimeData()->urls();
            for (const QUrl &url: urls) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo = QFileInfo(filePath);
                if (fileInfo.suffix().compare("xml", Qt::CaseInsensitive) != 0 &&
                    fileInfo.suffix().compare("mjb", Qt::CaseInsensitive) != 0) {
                    // Not an xml or mjb file, ignore it
                    event->ignore();
                    return;
                }
            }
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }

    void dropEvent(QDropEvent *event) override {
        const QMimeData *mimeData = event->mimeData();

        if (mimeData->hasUrls()) {
            QUrl url = mimeData->urls().first();
            QString filePath = url.toLocalFile();
            window->loadModel(filePath);
            event->acceptProposedAction();
        }
    }


    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched->inherits("QWindow")) {
            if (event->type() == QEvent::DragEnter) {
                // Forward the event to the dragEnterEvent handler of this widget
                auto dragEnterEvent = dynamic_cast<QDragEnterEvent *>(event);
                this->dragEnterEvent(dragEnterEvent);
                return true; // Event is handled
            } else if (event->type() == QEvent::Drop) {
                // Forward the event to the dropEvent handler of this widget
                auto dropEvent = dynamic_cast<QDropEvent *>(event);
                this->dropEvent(dropEvent);
                return true; // Event is handled
            } else if (event->type() == QEvent::KeyPress) {
                auto keyPressEvent = dynamic_cast<QKeyEvent *>(event);
                this->keyPressEvent(keyPressEvent);
                return true;
            }
        }
        return QWidget::eventFilter(watched, event);
    }

    void keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
            case Qt::Key_Plus:
                window->changeSlowDown(-1);
                break;
            case Qt::Key_Minus:
                window->changeSlowDown(1);
                break;

            case Qt::Key_Left:
                emit keyLeftPressed();
                break;
            case Qt::Key_Right:
                emit keyRightPressed();
                break;
            default:
                QWidget::keyPressEvent(event); // Call base class handler if you don't handle the event
        }
    }

private:
    MuJoCoOpenGLWindow *window;
};

#endif //QMUJOCOSIM_MY_WINDOW_CONTAINER_HPP
