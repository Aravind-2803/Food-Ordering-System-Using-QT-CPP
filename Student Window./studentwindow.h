#ifndef STUDENTWINDOW_H
#define STUDENTWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>

namespace Ui {
class studentwindow;
}

struct CartItem {
    int productId;
    QString productName;
    QString shopName;
    double price;
    int quantity;
    int shopId;
};

class StudentWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StudentWindow(int studentId, const QString &username, QWidget *parent = nullptr);
    ~StudentWindow();

private slots:
    void on_logoutButton_clicked();
    void on_placeOrderButton_clicked();
    void on_clearCartButton_clicked();
    void onAddToCartClicked();

private:
    Ui::studentwindow *ui;
    int studentId;
    QString username;
    QVector<CartItem> cartItems;

    void setupUI();
    void loadProducts();
    void loadProductsFromDatabase();
    void loadOrderHistory();
    void updateTotal();
    void clearCart();
};

#endif
