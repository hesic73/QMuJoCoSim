#ifndef QMUJOCOSIM_DIRECTORY_SELECTOR_HPP
#define QMUJOCOSIM_DIRECTORY_SELECTOR_HPP


#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QFrame>
#include <QMessageBox>
#include <QWidget>


class DirectorySelector : public QWidget {
Q_OBJECT

public:
    QLineEdit *directoryLineEdit;

    explicit DirectorySelector(const QString &labelText, QWidget *parent = nullptr) : QWidget(parent) {
        QHBoxLayout *layout = new QHBoxLayout(this);
        QLabel *label = new QLabel(labelText, this);
        directoryLineEdit = new QLineEdit(this);
        QPushButton *browseButton = new QPushButton("Browse...", this);

        layout->addWidget(label);
        layout->addWidget(directoryLineEdit);
        layout->addWidget(browseButton);

        connect(browseButton, &QPushButton::clicked, this, &DirectorySelector::selectDirectory);
        connect(directoryLineEdit, &QLineEdit::textChanged, this, &DirectorySelector::directoryChanged);
    }

    QString directory() const {
        return directoryLineEdit->text();
    }

    void setDirectory(const QString &directory) {
        directoryLineEdit->setText(directory);
    }

signals:

    void directoryChanged(const QString &newDirectory);

private slots:

    void selectDirectory() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", directoryLineEdit->text());
        if (!dir.isEmpty()) {
            directoryLineEdit->setText(dir);
        }
    }
};


#endif //QMUJOCOSIM_DIRECTORY_SELECTOR_HPP
