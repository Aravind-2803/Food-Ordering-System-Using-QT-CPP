#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QMessageBox>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance() {
        static DatabaseManager instance;
        return instance;
    }

    bool initializeDatabase();

    // User management
    bool registerUser(const QString &username, const QString &password,
                      const QString &userType, const QString &email = "",
                      const QString &phone = "");
    bool validateLogin(const QString &username, const QString &password, QString &userType);
    int getUserId(const QString &username);
    bool usernameExists(const QString &username);
    QString getUsername(int userId);

    // Shop management
    bool registerShop(int vendorId, const QString &shopName, const QString &slotNumber, const QString &description);
    int getShopId(int vendorId);
    QString getShopName(int shopId);
    QVector<QPair<int, QString>> getAllShops();

    // Product management
    bool addProduct(int shopId, const QString &name, double price, const QString &category);
    bool updateProductAvailability(int productId, bool available);
    QVector<QVector<QVariant>> getProductsByShop(int shopId);
    QVector<QVector<QVariant>> getAllAvailableProducts();

    // Order management
    int createOrder(int studentId, int shopId, double totalAmount);
    bool addOrderItem(int orderId, int productId, int quantity, double price);
    bool updateOrderStatus(int orderId, const QString &status);
    QVector<QVector<QVariant>> getOrdersByStudent(int studentId);
    QVector<QVector<QVariant>> getOrdersByShop(int shopId);
    QVector<QVector<QVariant>> getOrderItems(int orderId);

    // Financial queries
    double getTotalRevenue(int shopId);
    double getTodayRevenue(int shopId);
    int getTotalOrdersCount(int shopId);
    int getCompletedOrdersCount(int shopId);

private:
    DatabaseManager() {}
    ~DatabaseManager() {}
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};

#endif
