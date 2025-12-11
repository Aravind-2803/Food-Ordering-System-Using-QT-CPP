#ifndef VENDORWINDOW_H
#define VENDORWINDOW_H

#include <QMainWindow>

namespace Ui {
class vendorwindow;
}

class VendorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VendorWindow(int vendorId, const QString &username, QWidget *parent = nullptr);
    ~VendorWindow();

private slots:
    void on_logoutButton_clicked();
    void on_registerShopButton_clicked();
    void on_addProductButton_clicked();
    void onAcceptOrderClicked();
    void onCompleteOrderClicked();
    void onRemoveProductClicked();

private:
    Ui::vendorwindow *ui;
    int vendorId;
    QString username;
    int shopId;

    void setupUI();
    void loadMyProducts();
    void loadOrders();
    void loadFinancialData();
    bool validateShopRegistration();
    bool validateProductInput();
    void checkShopRegistration();
};

#endif
