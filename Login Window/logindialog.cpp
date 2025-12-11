#include "logindialog.h"
#include "ui_logindialog.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QFont>
#include <QPalette>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::logindialog)
{
    ui->setupUi(this);

    // Window settings
    setWindowTitle("CEG SQUARE - Login");
    setFixedSize(500, 450);

    // Connect buttons
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::on_loginButton_clicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::on_registerButton_clicked);

    // Connect Enter key to login
    connect(ui->passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::on_loginButton_clicked);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginButton_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString userType = ui->userTypeCombo->currentText().toLower();

    // Validation
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Input Error");
        msgBox.setText("Please enter both username and password.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        ui->usernameEdit->setFocus();
        return;
    }

    // Show loading state
    ui->loginButton->setText("Logging in...");
    ui->loginButton->setEnabled(false);
    ui->registerButton->setEnabled(false);
    QApplication::processEvents();

    QString actualUserType;
    if (DatabaseManager::instance().validateLogin(username, password, actualUserType)) {
        if (actualUserType.toLower() == userType.toLower()) {
            int userId = DatabaseManager::instance().getUserId(username);
            if (userId != -1) {
                emit loginSuccessful(userId, userType, username);
                // Don't close here, let main.cpp handle the transition
            } else {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Login Error");
                msgBox.setText("Failed to get user information.");
                msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.exec();
            }
        } else {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Login Error");
            msgBox.setText(QString("You are registered as a %1, but selected %2.")
                               .arg(actualUserType).arg(userType));
            msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        }
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Login Failed");
        msgBox.setText("Invalid username or password.\n\n"
                       "Sample accounts:\n"
                       "Student: student1 / pass123\n"
                       "Vendor: vendor1 / pass123");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    // Reset button
    ui->loginButton->setText("LOGIN");
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
}

void LoginDialog::on_registerButton_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString userType = ui->userTypeCombo->currentText().toLower();

    // Validation
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Input Error");
        msgBox.setText("Please enter both username and password.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        ui->usernameEdit->setFocus();
        return;
    }

    if (password.length() < 4) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Input Error");
        msgBox.setText("Password must be at least 4 characters long.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        ui->passwordEdit->setFocus();
        ui->passwordEdit->selectAll();
        return;
    }

    // Show loading state
    ui->registerButton->setText("Registering...");
    ui->registerButton->setEnabled(false);
    ui->loginButton->setEnabled(false);
    QApplication::processEvents();

    // Just try to register - the database will handle the duplicate check
    if (DatabaseManager::instance().registerUser(username, password, userType)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Registration Successful");
        msgBox.setText("Account created successfully! You can now login.");
        msgBox.setStyleSheet("QLabel{color: #2E7D32; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();

        ui->passwordEdit->clear();
        ui->usernameEdit->setFocus();
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Registration Failed");
        msgBox.setText("Username already exists. Please try a different username.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        ui->usernameEdit->setFocus();
        ui->usernameEdit->selectAll();
    }

    // Reset button
    ui->registerButton->setText("REGISTER");
    ui->registerButton->setEnabled(true);
    ui->loginButton->setEnabled(true);
}
