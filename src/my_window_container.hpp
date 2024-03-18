#ifndef MUJOCO_SIMULATION_QT_MY_WINDOW_CONTAINER_HPP
#define MUJOCO_SIMULATION_QT_MY_WINDOW_CONTAINER_HPP

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

protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasUrls()) {
            QList<QUrl> urls = event->mimeData()->urls();
            for (const QUrl &url: urls) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo = QFileInfo(filePath);
                if (fileInfo.suffix().compare("xml", Qt::CaseInsensitive) != 0) {
                    // Not an xml file, ignore it
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
            }
        }
        return QWidget::eventFilter(watched, event);
    }

private:
    MuJoCoOpenGLWindow *window;
};

#endif //MUJOCO_SIMULATION_QT_MY_WINDOW_CONTAINER_HPP
