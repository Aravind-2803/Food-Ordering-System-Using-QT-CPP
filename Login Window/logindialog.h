#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class logindialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

signals:
    void loginSuccessful(int userId, const QString &userType, const QString &username);

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();

private:
    Ui::logindialog *ui;
};

#endif
