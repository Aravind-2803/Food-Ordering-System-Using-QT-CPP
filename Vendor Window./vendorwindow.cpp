#include "vendorwindow.h"
#include "ui_vendorwindow.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QDateTime>

VendorWindow::VendorWindow(int vendorId, const QString &username, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::vendorwindow),
    vendorId(vendorId),
    username(username),
    shopId(-1)
{
    ui->setupUi(this);
    setupUI();
}

VendorWindow::~VendorWindow()
{
    delete ui;
}

void VendorWindow::setupUI()
{
    setWindowTitle("CEG SQUARE - Vendor Portal - Welcome " + username);
    ui->headerLabel->setText("Vendor Dashboard - " + username);

    // Setup table properties
    ui->productsTable->horizontalHeader()->setStretchLastSection(true);
    ui->ordersTable->horizontalHeader()->setStretchLastSection(true);
    ui->paymentHistoryTable->horizontalHeader()->setStretchLastSection(true);

    // Connect signals
    connect(ui->logoutButton, &QPushButton::clicked, this, &VendorWindow::on_logoutButton_clicked);
    connect(ui->registerShopButton, &QPushButton::clicked, this, &VendorWindow::on_registerShopButton_clicked);
    connect(ui->addProductButton, &QPushButton::clicked, this, &VendorWindow::on_addProductButton_clicked);

    // Check if shop already registered
    checkShopRegistration();

    // Load initial data
    loadMyProducts();
    loadOrders();
    loadFinancialData();
}

void VendorWindow::checkShopRegistration()
{
    shopId = DatabaseManager::instance().getShopId(vendorId);
    if (shopId != -1) {
        QString shopName = DatabaseManager::instance().getShopName(shopId);
        ui->shopStatusLabel->setText("Shop Registered: " + shopName);
        ui->shopStatusLabel->setStyleSheet("color: #4CAF50; font-weight: bold; padding: 10px;");
        ui->shopNameEdit->setEnabled(false);
        ui->slotEdit->setEnabled(false);
        ui->descriptionEdit->setEnabled(false);
        ui->registerShopButton->setEnabled(false);
    } else {
        ui->shopStatusLabel->setText("No shop registered");
        ui->shopStatusLabel->setStyleSheet("color: #f44336; font-weight: bold; padding: 10px;");
    }
}

void VendorWindow::on_logoutButton_clicked()
{
    this->close();
}

void VendorWindow::on_registerShopButton_clicked()
{
    if (!validateShopRegistration()) {
        return;
    }

    QString shopName = ui->shopNameEdit->text().trimmed();
    QString slotNumber = ui->slotEdit->text().trimmed().toUpper();
    QString description = ui->descriptionEdit->toPlainText().trimmed();

    if (DatabaseManager::instance().registerShop(vendorId, shopName, slotNumber, description)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Success");
        msgBox.setText(QString("Shop '%1' registered successfully at slot %2!").arg(shopName).arg(slotNumber));
        msgBox.setStyleSheet("QLabel{color: #2E7D32; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();

        // Update UI
        checkShopRegistration();

        // Clear form
        ui->shopNameEdit->clear();
        ui->slotEdit->clear();
        ui->descriptionEdit->clear();
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Registration Failed");
        msgBox.setText("Failed to register shop. Please try again.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

void VendorWindow::on_addProductButton_clicked()
{
    if (shopId == -1) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("No Shop");
        msgBox.setText("Please register a shop first before adding products.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }

    if (!validateProductInput()) {
        return;
    }

    QString productName = ui->productNameEdit->text().trimmed();
    double price = ui->priceEdit->text().toDouble();
    QString category = ui->categoryEdit->text().trimmed();

    if (DatabaseManager::instance().addProduct(shopId, productName, price, category)) {
        // Reload products
        loadMyProducts();

        // Clear form
        ui->productNameEdit->clear();
        ui->priceEdit->clear();
        ui->categoryEdit->clear();

        statusBar()->showMessage("Product added successfully!", 3000);
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("Failed to add product. Please try again.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

void VendorWindow::onRemoveProductClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int productId = button->property("productId").toInt();
    int row = button->property("row").toInt();

    if (DatabaseManager::instance().updateProductAvailability(productId, false)) {
        ui->productsTable->removeRow(row);
        statusBar()->showMessage("Product removed!", 3000);
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("Failed to remove product.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

void VendorWindow::onAcceptOrderClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int orderId = button->property("orderId").toInt();
    int row = button->property("row").toInt();

    if (DatabaseManager::instance().updateOrderStatus(orderId, "preparing")) {
        ui->ordersTable->item(row, 4)->setText("Preparing");
        button->setEnabled(false);
        statusBar()->showMessage("Order accepted and now being prepared!", 3000);
        loadOrders(); // Reload to update statistics
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("Failed to update order status.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

void VendorWindow::onCompleteOrderClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int orderId = button->property("orderId").toInt();
    int row = button->property("row").toInt();

    if (DatabaseManager::instance().updateOrderStatus(orderId, "completed")) {
        ui->ordersTable->item(row, 4)->setText("Completed");
        button->setEnabled(false);
        statusBar()->showMessage("Order marked as completed!", 3000);
        loadOrders(); // Reload to update statistics
        loadFinancialData(); // Reload financial data
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("Failed to update order status.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

bool VendorWindow::validateShopRegistration()
{
    if (ui->shopNameEdit->text().trimmed().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Validation Error");
        msgBox.setText("Please enter a shop name.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }

    if (ui->slotEdit->text().trimmed().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Validation Error");
        msgBox.setText("Please enter a slot number.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }

    return true;
}

bool VendorWindow::validateProductInput()
{
    if (ui->productNameEdit->text().trimmed().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Validation Error");
        msgBox.setText("Please enter a product name.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }

    if (ui->priceEdit->text().trimmed().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Validation Error");
        msgBox.setText("Please enter a price.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }

    bool ok;
    double price = ui->priceEdit->text().toDouble(&ok);
    if (!ok || price <= 0) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Validation Error");
        msgBox.setText("Please enter a valid price.");
        msgBox.setStyleSheet("QLabel{color: #B71C1C; font-weight: bold;} QPushButton{ padding: 5px 10px; }");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }

    return true;
}

void VendorWindow::loadMyProducts()
{
    // Clear existing data
    ui->productsTable->setRowCount(0);

    if (shopId == -1) {
        return;
    }

    auto products = DatabaseManager::instance().getProductsByShop(shopId);

    for (int i = 0; i < products.size(); ++i) {
        const auto& product = products[i];
        int row = ui->productsTable->rowCount();
        ui->productsTable->insertRow(row);

        int productId = product[0].toInt();
        QString productName = product[1].toString();
        double price = product[2].toDouble();
        QString category = product[3].toString();
        bool available = product[4].toBool();

        ui->productsTable->setItem(row, 0, new QTableWidgetItem(productName));
        ui->productsTable->setItem(row, 1, new QTableWidgetItem(QString("₹%1").arg(price, 0, 'f', 2)));
        ui->productsTable->setItem(row, 2, new QTableWidgetItem(category));
        ui->productsTable->setItem(row, 3, new QTableWidgetItem(available ? "Available" : "Not Available"));

        // Remove button
        QPushButton *removeButton = new QPushButton("Remove");
        removeButton->setProperty("productId", productId);
        removeButton->setProperty("row", row);
        removeButton->setStyleSheet("background-color: #f44336; color: white; padding: 4px;");
        ui->productsTable->setCellWidget(row, 4, removeButton);

        connect(removeButton, &QPushButton::clicked, this, &VendorWindow::onRemoveProductClicked);
    }
}

void VendorWindow::loadOrders()
{
    // Clear existing data
    ui->ordersTable->setRowCount(0);

    if (shopId == -1) {
        return;
    }

    auto orders = DatabaseManager::instance().getOrdersByShop(shopId);

    int pendingCount = 0, preparingCount = 0, completedCount = 0;

    for (int i = 0; i < orders.size(); ++i) {
        const auto& order = orders[i];
        int row = ui->ordersTable->rowCount();
        ui->ordersTable->insertRow(row);

        int orderId = order[0].toInt();
        QString customer = order[1].toString();
        double total = order[2].toDouble();
        QString status = order[3].toString();
        QDateTime orderDateTime = order[4].toDateTime();
        QString orderDate = orderDateTime.toString("hh:mm AP");
        QString items = order[5].toString();

        // Count status
        if (status == "pending") pendingCount++;
        else if (status == "preparing") preparingCount++;
        else if (status == "completed") completedCount++;

        ui->ordersTable->setItem(row, 0, new QTableWidgetItem(QString::number(orderId)));
        ui->ordersTable->setItem(row, 1, new QTableWidgetItem(customer));
        ui->ordersTable->setItem(row, 2, new QTableWidgetItem(items));
        ui->ordersTable->setItem(row, 3, new QTableWidgetItem(QString("₹%1").arg(total, 0, 'f', 2)));
        ui->ordersTable->setItem(row, 4, new QTableWidgetItem(status));
        ui->ordersTable->setItem(row, 5, new QTableWidgetItem(orderDate));

        // Add action buttons
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(actionWidget);

        QPushButton *acceptButton = new QPushButton("Accept");
        QPushButton *completeButton = new QPushButton("Complete");

        acceptButton->setProperty("orderId", orderId);
        acceptButton->setProperty("row", row);
        completeButton->setProperty("orderId", orderId);
        completeButton->setProperty("row", row);

        acceptButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 4px;");
        completeButton->setStyleSheet("background-color: #2196F3; color: white; padding: 4px;");

        // Disable buttons based on status
        if (status == "preparing" || status == "completed") {
            acceptButton->setEnabled(false);
        }
        if (status == "completed") {
            completeButton->setEnabled(false);
        }

        layout->addWidget(acceptButton);
        layout->addWidget(completeButton);
        layout->setContentsMargins(2, 2, 2, 2);

        ui->ordersTable->setCellWidget(row, 6, actionWidget);

        // Connect buttons
        connect(acceptButton, &QPushButton::clicked, this, &VendorWindow::onAcceptOrderClicked);
        connect(completeButton, &QPushButton::clicked, this, &VendorWindow::onCompleteOrderClicked);
    }

    // Update order statistics
    ui->pendingOrdersLabel->setText(QString("Pending: %1").arg(pendingCount));
    ui->preparingOrdersLabel->setText(QString("Preparing: %1").arg(preparingCount));
    ui->readyOrdersLabel->setText(QString("Completed: %1").arg(completedCount));
}

void VendorWindow::loadFinancialData()
{
    if (shopId == -1) {
        ui->totalRevenueLabel->setText("Total Revenue: ₹0.00");
        ui->todayRevenueLabel->setText("Today's Revenue: ₹0.00");
        ui->totalOrdersLabel->setText("Total Orders: 0");
        ui->completedOrdersLabel->setText("Completed Orders: 0");
        return;
    }

    double totalRevenue = DatabaseManager::instance().getTotalRevenue(shopId);
    double todayRevenue = DatabaseManager::instance().getTodayRevenue(shopId);
    int totalOrders = DatabaseManager::instance().getTotalOrdersCount(shopId);
    int completedOrders = DatabaseManager::instance().getCompletedOrdersCount(shopId);

    ui->totalRevenueLabel->setText(QString("Total Revenue: ₹%1").arg(totalRevenue, 0, 'f', 2));
    ui->todayRevenueLabel->setText(QString("Today's Revenue: ₹%1").arg(todayRevenue, 0, 'f', 2));
    ui->totalOrdersLabel->setText(QString("Total Orders: %1").arg(totalOrders));
    ui->completedOrdersLabel->setText(QString("Completed Orders: %1").arg(completedOrders));

    // Load payment history (completed orders)
    auto orders = DatabaseManager::instance().getOrdersByShop(shopId);
    ui->paymentHistoryTable->setRowCount(0);

    int historyCount = 0;
    for (const auto& order : orders) {
        if (order[3].toString() == "completed") {
            int row = ui->paymentHistoryTable->rowCount();
            ui->paymentHistoryTable->insertRow(row);

            QDateTime orderDateTime = order[4].toDateTime();
            ui->paymentHistoryTable->setItem(row, 0, new QTableWidgetItem(orderDateTime.toString("yyyy-MM-dd")));
            ui->paymentHistoryTable->setItem(row, 1, new QTableWidgetItem(order[0].toString()));
            ui->paymentHistoryTable->setItem(row, 2, new QTableWidgetItem(QString("₹%1").arg(order[2].toDouble(), 0, 'f', 2)));
            ui->paymentHistoryTable->setItem(row, 3, new QTableWidgetItem("Completed"));

            historyCount++;
            if (historyCount >= 10) break; // Show only last 10
        }
    }
}
