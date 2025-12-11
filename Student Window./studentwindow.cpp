#include "studentwindow.h"
#include "ui_studentwindow.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDate>
#include <QDateTime>

StudentWindow::StudentWindow(int studentId, const QString &username, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::studentwindow),
    studentId(studentId),
    username(username)
{
    ui->setupUi(this);
    setupUI();
}

StudentWindow::~StudentWindow()
{
    delete ui;
}

void StudentWindow::setupUI()
{
    setWindowTitle("CEG SQUARE - Student Portal - Welcome " + username);
    ui->welcomeLabel->setText("Welcome, " + username + "! Order food from CEG canteen");

    // Setup table properties
    ui->productsTable->setColumnCount(5);
    ui->productsTable->setHorizontalHeaderLabels({"Product", "Shop", "Price (₹)", "Quantity", "Add to Cart"});
    ui->productsTable->horizontalHeader()->setStretchLastSection(true);

    ui->historyTable->setColumnCount(5);
    ui->historyTable->setHorizontalHeaderLabels({"Order ID", "Shop", "Total", "Status", "Date"});
    ui->historyTable->horizontalHeader()->setStretchLastSection(true);

    // Connect signals
    connect(ui->logoutButton, &QPushButton::clicked, this, &StudentWindow::on_logoutButton_clicked);
    connect(ui->placeOrderButton, &QPushButton::clicked, this, &StudentWindow::on_placeOrderButton_clicked);
    connect(ui->clearCartButton, &QPushButton::clicked, this, &StudentWindow::on_clearCartButton_clicked);

    // Load data
    loadProducts();
    loadOrderHistory();
}

void StudentWindow::loadProducts()
{
    loadProductsFromDatabase();
}

void StudentWindow::loadProductsFromDatabase()
{
    // Clear existing data
    ui->productsTable->setRowCount(0);

    auto products = DatabaseManager::instance().getAllAvailableProducts();

    if (products.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("No Products");
        msgBox.setText("No products available at the moment.");
        msgBox.setStyleSheet("QLabel{color: #1976D2; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        return;
    }

    for (int row = 0; row < products.size(); ++row) {
        const auto& product = products[row];
        ui->productsTable->insertRow(row);

        int productId = product[0].toInt();
        QString productName = product[1].toString();
        QString shopName = product[2].toString();
        double price = product[3].toDouble();
        int shopId = product[6].toInt();

        ui->productsTable->setItem(row, 0, new QTableWidgetItem(productName));
        ui->productsTable->setItem(row, 1, new QTableWidgetItem(shopName));
        ui->productsTable->setItem(row, 2, new QTableWidgetItem(QString("₹%1").arg(price, 0, 'f', 2)));

        // Quantity spin box
        QSpinBox *quantitySpin = new QSpinBox();
        quantitySpin->setMinimum(1);
        quantitySpin->setMaximum(10);
        quantitySpin->setValue(1);
        ui->productsTable->setCellWidget(row, 3, quantitySpin);

        // Add to cart button
        QPushButton *addButton = new QPushButton("Add to Cart");
        addButton->setProperty("productId", productId);
        addButton->setProperty("productName", productName);
        addButton->setProperty("price", price);
        addButton->setProperty("shopName", shopName);
        addButton->setProperty("shopId", shopId);
        addButton->setProperty("row", row);
        addButton->setStyleSheet("background-color: #2196F3; color: white; padding: 4px;");
        ui->productsTable->setCellWidget(row, 4, addButton);

        connect(addButton, &QPushButton::clicked, this, &StudentWindow::onAddToCartClicked);
    }

    qDebug() << "Loaded" << products.size() << "products from database";
}

void StudentWindow::onAddToCartClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int productId = button->property("productId").toInt();
    QString productName = button->property("productName").toString();
    double price = button->property("price").toDouble();
    QString shopName = button->property("shopName").toString();
    int shopId = button->property("shopId").toInt();
    int row = button->property("row").toInt();

    QSpinBox *spinBox = qobject_cast<QSpinBox*>(ui->productsTable->cellWidget(row, 3));
    if (spinBox) {
        int quantity = spinBox->value();

        // Check if product already in cart
        bool found = false;
        for (auto& item : cartItems) {
            if (item.productId == productId) {
                item.quantity += quantity;
                found = true;
                break;
            }
        }

        if (!found) {
            CartItem newItem;
            newItem.productId = productId;
            newItem.productName = productName;
            newItem.shopName = shopName;
            newItem.price = price;
            newItem.quantity = quantity;
            newItem.shopId = shopId;
            cartItems.append(newItem);
        }

        // Update cart display
        ui->cartList->clear();
        for (const auto& item : cartItems) {
            double totalPrice = item.quantity * item.price;
            QString cartItem = QString("%1 x %2 - ₹%3 (%4)")
                                   .arg(item.quantity)
                                   .arg(item.productName)
                                   .arg(totalPrice, 0, 'f', 2)
                                   .arg(item.shopName);
            ui->cartList->addItem(cartItem);
        }

        updateTotal();

        // Reset quantity to 1
        spinBox->setValue(1);

        // Show brief status message instead of dialog
        statusBar()->showMessage(QString("Added %1 x %2 to cart!").arg(quantity).arg(productName), 3000);
    }
}

void StudentWindow::loadOrderHistory()
{
    // Clear existing data
    ui->historyTable->setRowCount(0);

    auto orders = DatabaseManager::instance().getOrdersByStudent(studentId);

    for (int i = 0; i < orders.size(); ++i) {
        const auto& order = orders[i];
        int row = ui->historyTable->rowCount();
        ui->historyTable->insertRow(row);

        ui->historyTable->setItem(row, 0, new QTableWidgetItem(order[0].toString()));
        ui->historyTable->setItem(row, 1, new QTableWidgetItem(order[1].toString()));
        ui->historyTable->setItem(row, 2, new QTableWidgetItem(QString("₹%1").arg(order[2].toDouble(), 0, 'f', 2)));

        QTableWidgetItem *statusItem = new QTableWidgetItem(order[3].toString());
        // Color code status
        if (order[3].toString() == "completed") {
            statusItem->setBackground(QColor(200, 255, 200));
        } else if (order[3].toString() == "preparing") {
            statusItem->setBackground(QColor(255, 255, 200));
        } else if (order[3].toString() == "pending") {
            statusItem->setBackground(QColor(255, 200, 200));
        }
        ui->historyTable->setItem(row, 3, statusItem);

        ui->historyTable->setItem(row, 4, new QTableWidgetItem(order[4].toDateTime().toString("yyyy-MM-dd hh:mm")));
    }
}

void StudentWindow::updateTotal()
{
    double total = 0.0;
    for (const auto& item : cartItems) {
        total += item.quantity * item.price;
    }
    ui->totalLabel->setText(QString("Total: ₹%1").arg(total, 0, 'f', 2));
}

void StudentWindow::clearCart()
{
    cartItems.clear();
    ui->cartList->clear();
    updateTotal();
}

void StudentWindow::on_logoutButton_clicked()
{
    this->close();
}

void StudentWindow::on_placeOrderButton_clicked()
{
    if (cartItems.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Cart Empty");
        msgBox.setText("Your cart is empty! Add some items first.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }

    // Check if all items are from the same shop
    int firstShopId = cartItems[0].shopId;
    for (const auto& item : cartItems) {
        if (item.shopId != firstShopId) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Multiple Shops");
            msgBox.setText("Please order from one shop at a time. Your cart contains items from multiple shops.");
            msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return;
        }
    }

    // Calculate total
    double total = 0.0;
    for (const auto& item : cartItems) {
        total += item.quantity * item.price;
    }

    // Create order in database
    int orderId = DatabaseManager::instance().createOrder(studentId, firstShopId, total);
    if (orderId == -1) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Order Failed");
        msgBox.setText("Failed to create order. Please try again.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    // Add order items
    bool allItemsAdded = true;
    for (const auto& item : cartItems) {
        if (!DatabaseManager::instance().addOrderItem(orderId, item.productId, item.quantity, item.price)) {
            allItemsAdded = false;
        }
    }

    if (allItemsAdded) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Order Placed");
        msgBox.setText(QString("Order #%1 placed successfully!\nTotal Amount: ₹%2\nShop: %3")
                           .arg(orderId)
                           .arg(total, 0, 'f', 2)
                           .arg(cartItems[0].shopName));
        msgBox.setStyleSheet("QLabel{color: #2E7D32; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();

        // Clear cart and reload history
        clearCart();
        loadOrderHistory();
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Order Partially Failed");
        msgBox.setText("Order was placed but some items may not have been added correctly.");
        msgBox.setStyleSheet("QLabel{color: #FF6F00; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

void StudentWindow::on_clearCartButton_clicked()
{
    if (cartItems.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Cart Empty");
        msgBox.setText("Your cart is already empty!");
        msgBox.setStyleSheet("QLabel{color: #1976D2; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
        return;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Clear Cart");
    msgBox.setText("Are you sure you want to clear your cart?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
    msgBox.setIcon(QMessageBox::Question);

    if (msgBox.exec() == QMessageBox::Yes) {
        clearCart();
        statusBar()->showMessage("Your cart has been cleared!", 3000);
    }
}
