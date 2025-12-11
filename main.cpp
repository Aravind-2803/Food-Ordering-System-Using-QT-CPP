#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include "databasemanager.h"
#include "logindialog.h"
#include "studentwindow.h"
#include "vendorwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application font
    QFont appFont("Segoe UI", 9);
    app.setFont(appFont);

    // Set application info
    app.setApplicationName("CEG SQUARE");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("CEG");

    // Initialize database
    qDebug() << "Initializing database...";
    if (!DatabaseManager::instance().initializeDatabase()) {
        QMessageBox::critical(nullptr, "Database Error",
                              "Failed to initialize database!\n\n"
                              "Please check if the application has write permissions "
                              "or if another instance is running.");
        return -1;
    }

    qDebug() << "Database initialized successfully";

    LoginDialog loginDialog;
    QMainWindow *currentWindow = nullptr;

    QObject::connect(&loginDialog, &LoginDialog::loginSuccessful,
                     [&](int userId, const QString &userType, const QString &username) {
                         qDebug() << "Login successful - User:" << username << "Type:" << userType << "ID:" << userId;

                         // Delete previous window if exists
                         if (currentWindow) {
                             currentWindow->deleteLater();
                             currentWindow = nullptr;
                         }

                         if (userType.toLower() == "student") {
                             currentWindow = new StudentWindow(userId, username);
                         } else if (userType.toLower() == "vendor") {
                             currentWindow = new VendorWindow(userId, username);
                         } else {
                             QMessageBox::critical(nullptr, "Error", "Unknown user type: " + userType);
                             return;
                         }

                         if (currentWindow) {
                             currentWindow->show();
                             loginDialog.hide();

                             // Connect window close to show login again
                             QObject::connect(currentWindow, &QMainWindow::destroyed, [&]() {
                                 currentWindow = nullptr;
                                 loginDialog.show();
                                 loginDialog.raise();
                                 loginDialog.activateWindow();
                             });
                         }
                     });

    loginDialog.show();

    int result = app.exec();

    // Cleanup
    if (currentWindow) {
        currentWindow->deleteLater();
    }

    return result;
}
