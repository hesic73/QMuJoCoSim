#ifndef QMUJOCOSIM_SETTINGS_DIALOG_HPP
#define QMUJOCOSIM_SETTINGS_DIALOG_HPP


#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include "custom_widgets/directory_selector.hpp"  // Assuming DirectorySelector is in a separate header

class SettingsDialog : public QDialog {
Q_OBJECT

private:
    QPushButton *saveButton;
    DirectorySelector *xmlModelDirectorySelector;
    DirectorySelector *mjbModelDirectorySelector;
    DirectorySelector *printModelDirectorySelector;
    DirectorySelector *printDataDirectorySelector;
    DirectorySelector *screenshotDirectorySelector;
    QSettings &settings;

    const QString defaultButtonStyle = "QPushButton { background-color: white; }";
    const QString modifiedButtonStyle = "QPushButton { background-color: #4dabf7; }"; // Vibrant blue color

public:
    explicit SettingsDialog(QSettings &settings, QWidget *parent = nullptr)
            : QDialog(parent), settings(settings) {
        setWindowTitle("Settings");
        setMinimumSize(600, 300);


        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QFrame *frame = new QFrame(this);
        frame->setFrameShape(QFrame::StyledPanel);
        QVBoxLayout *frameLayout = new QVBoxLayout(frame);
        frameLayout->setAlignment(Qt::AlignTop);

        // Initialize directory selectors

        initializeDirectorySelector(xmlModelDirectorySelector, "XML Model Directory:", "xml_model_directory");
        initializeDirectorySelector(mjbModelDirectorySelector, "MJB Model Directory:", "mjb_model_directory");
        initializeDirectorySelector(printModelDirectorySelector, "Print Model Directory:", "print_model_directory");
        initializeDirectorySelector(printDataDirectorySelector, "Print Data Directory:", "print_data_directory");
        initializeDirectorySelector(screenshotDirectorySelector, "Screenshot Directory:", "screenshot_directory");

        // Add selectors to frame layout
        frameLayout->addWidget(xmlModelDirectorySelector);
        frameLayout->addWidget(mjbModelDirectorySelector);
        frameLayout->addWidget(printModelDirectorySelector);
        frameLayout->addWidget(printDataDirectorySelector);
        frameLayout->addWidget(screenshotDirectorySelector);
        mainLayout->addWidget(frame);

        // Buttons for saving and closing
        auto *buttonLayout = new QHBoxLayout();
        saveButton = new QPushButton("Save", this);
        saveButton->setStyleSheet(defaultButtonStyle);
        auto *closeButton = new QPushButton("Close", this);
        connect(saveButton, &QPushButton::clicked, [this]() {
            if (saveSettings()) {
                saveButton->setStyleSheet(defaultButtonStyle); // Reset button color when settings are saved
            }
        });
        connect(closeButton, &QPushButton::clicked, this, &SettingsDialog::reject);

        buttonLayout->addStretch(1);
        buttonLayout->addWidget(saveButton);
        buttonLayout->addWidget(closeButton);
        buttonLayout->addStretch(1);

        mainLayout->addLayout(buttonLayout);
    }


private:

    bool saveSettings() {
        bool allSaved = true;
        allSaved &= saveDirectorySetting(xmlModelDirectorySelector, "xml_model_directory");
        allSaved &= saveDirectorySetting(mjbModelDirectorySelector, "mjb_model_directory");
        allSaved &= saveDirectorySetting(printModelDirectorySelector, "print_model_directory");
        allSaved &= saveDirectorySetting(printDataDirectorySelector, "print_data_directory");
        allSaved &= saveDirectorySetting(screenshotDirectorySelector, "screenshot_directory");
        return allSaved;
    }

    bool saveDirectorySetting(DirectorySelector *selector, const QString &settingKey) {
        QDir dir(selector->directory());
        if (dir.exists()) {
            settings.setValue(settingKey, selector->directory());
            return true;
        } else {
            QMessageBox::warning(this, "Invalid Directory",
                                 QString("The directory path for %1 entered is invalid. Please enter a valid directory path.").arg(
                                         selector->directory()));
            return false;
        }
    }


    void initializeDirectorySelector(DirectorySelector *&selector, const QString &label, const QString &settingKey) {
        selector = new DirectorySelector(label, this);
        selector->setDirectory(settings.value(settingKey, QDir::currentPath()).toString());
        connect(selector, &DirectorySelector::directoryChanged, [this]() {
            saveButton->setStyleSheet(modifiedButtonStyle);
        });
    }

};


#endif //QMUJOCOSIM_SETTINGS_DIALOG_HPP
