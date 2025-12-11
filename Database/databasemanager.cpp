#include "databasemanager.h"
#include <QDebug>

bool DatabaseManager::initializeDatabase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("ceg_square.db");

    if (!db.open()) {
        qDebug() << "Error: connection with database failed:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;

    // Users table
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "username TEXT UNIQUE NOT NULL, "
                    "password TEXT NOT NULL, "
                    "user_type TEXT CHECK(user_type IN ('student', 'vendor')) NOT NULL, "
                    "email TEXT, "
                    "phone TEXT)")) {
        qDebug() << "Create users table error:" << query.lastError().text();
        return false;
    }

    // Shops table
    if (!query.exec("CREATE TABLE IF NOT EXISTS shops ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "vendor_id INTEGER, "
                    "shop_name TEXT NOT NULL, "
                    "slot_number TEXT, "
                    "description TEXT, "
                    "rent_status TEXT DEFAULT 'occupied', "
                    "FOREIGN KEY(vendor_id) REFERENCES users(id))")) {
        qDebug() << "Create shops table error:" << query.lastError().text();
        return false;
    }

    // Products table
    if (!query.exec("CREATE TABLE IF NOT EXISTS products ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "shop_id INTEGER, "
                    "name TEXT NOT NULL, "
                    "price REAL NOT NULL, "
                    "category TEXT, "
                    "available BOOLEAN DEFAULT 1, "
                    "FOREIGN KEY(shop_id) REFERENCES shops(id))")) {
        qDebug() << "Create products table error:" << query.lastError().text();
        return false;
    }

    // Orders table
    if (!query.exec("CREATE TABLE IF NOT EXISTS orders ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "student_id INTEGER, "
                    "shop_id INTEGER, "
                    "total_amount REAL NOT NULL, "
                    "status TEXT DEFAULT 'pending', "
                    "order_date DATETIME DEFAULT CURRENT_TIMESTAMP, "
                    "FOREIGN KEY(student_id) REFERENCES users(id), "
                    "FOREIGN KEY(shop_id) REFERENCES shops(id))")) {
        qDebug() << "Create orders table error:" << query.lastError().text();
        return false;
    }

    // Order items table
    if (!query.exec("CREATE TABLE IF NOT EXISTS order_items ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "order_id INTEGER, "
                    "product_id INTEGER, "
                    "quantity INTEGER, "
                    "price REAL, "
                    "FOREIGN KEY(order_id) REFERENCES orders(id), "
                    "FOREIGN KEY(product_id) REFERENCES products(id))")) {
        qDebug() << "Create order_items table error:" << query.lastError().text();
        return false;
    }

    // Add sample data only if tables are empty
    query.exec("INSERT OR IGNORE INTO users (username, password, user_type) VALUES "
               "('student1', 'pass123', 'student'), "
               "('vendor1', 'pass123', 'vendor')");

    // Check if vendor exists before adding shop
    query.exec("INSERT OR IGNORE INTO shops (vendor_id, shop_name, slot_number, description) "
               "SELECT id, 'Biryani Corner', 'A1', 'Best biryani in town' FROM users WHERE username='vendor1'");

    // Add products only if they don't exist
    query.exec("INSERT OR IGNORE INTO products (shop_id, name, price, category) "
               "SELECT 1, 'Chicken Biryani', 120.0, 'Main Course' WHERE NOT EXISTS "
               "(SELECT 1 FROM products WHERE name='Chicken Biryani')");

    query.exec("INSERT OR IGNORE INTO products (shop_id, name, price, category) "
               "SELECT 1, 'Veg Biryani', 80.0, 'Main Course' WHERE NOT EXISTS "
               "(SELECT 1 FROM products WHERE name='Veg Biryani')");

    query.exec("INSERT OR IGNORE INTO products (shop_id, name, price, category) "
               "SELECT 1, 'Coke', 20.0, 'Beverages' WHERE NOT EXISTS "
               "(SELECT 1 FROM products WHERE name='Coke')");

    return true;
}

bool DatabaseManager::registerUser(const QString &username, const QString &password,
                                   const QString &userType, const QString &email,
                                   const QString &phone) {
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, user_type, email, phone) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(userType.toLower());
    query.addBindValue(email);
    query.addBindValue(phone);

    bool success = query.exec();
    if (!success) {
        QString error = query.lastError().text();
        qDebug() << "Registration error:" << error;
        if (error.contains("UNIQUE constraint failed") || error.contains("duplicate")) {
            return false;
        }
    }
    return success;
}

bool DatabaseManager::validateLogin(const QString &username, const QString &password, QString &userType) {
    QSqlQuery query;
    query.prepare("SELECT user_type FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);

    if (query.exec() && query.next()) {
        userType = query.value(0).toString();
        return true;
    }

    qDebug() << "Login failed for user:" << username << "Error:" << query.lastError().text();
    return false;
}

int DatabaseManager::getUserId(const QString &username) {
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    qDebug() << "Get user ID failed for:" << username;
    return -1;
}

bool DatabaseManager::usernameExists(const QString &username) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

QString DatabaseManager::getUsername(int userId) {
    QSqlQuery query;
    query.prepare("SELECT username FROM users WHERE id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "";
}

bool DatabaseManager::registerShop(int vendorId, const QString &shopName, const QString &slotNumber, const QString &description) {
    QSqlQuery query;
    query.prepare("INSERT INTO shops (vendor_id, shop_name, slot_number, description) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(vendorId);
    query.addBindValue(shopName);
    query.addBindValue(slotNumber);
    query.addBindValue(description);

    bool success = query.exec();
    if (!success) {
        qDebug() << "Shop registration error:" << query.lastError().text();
    }
    return success;
}

int DatabaseManager::getShopId(int vendorId) {
    QSqlQuery query;
    query.prepare("SELECT id FROM shops WHERE vendor_id = ?");
    query.addBindValue(vendorId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return -1;
}

QString DatabaseManager::getShopName(int shopId) {
    QSqlQuery query;
    query.prepare("SELECT shop_name FROM shops WHERE id = ?");
    query.addBindValue(shopId);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "";
}

QVector<QPair<int, QString>> DatabaseManager::getAllShops() {
    QVector<QPair<int, QString>> shops;
    QSqlQuery query("SELECT id, shop_name FROM shops WHERE rent_status = 'occupied'");

    while (query.next()) {
        shops.append(qMakePair(query.value(0).toInt(), query.value(1).toString()));
    }
    return shops;
}

bool DatabaseManager::addProduct(int shopId, const QString &name, double price, const QString &category) {
    QSqlQuery query;
    query.prepare("INSERT INTO products (shop_id, name, price, category) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(shopId);
    query.addBindValue(name);
    query.addBindValue(price);
    query.addBindValue(category);

    bool success = query.exec();
    if (!success) {
        qDebug() << "Add product error:" << query.lastError().text();
    }
    return success;
}

bool DatabaseManager::updateProductAvailability(int productId, bool available) {
    QSqlQuery query;
    query.prepare("UPDATE products SET available = ? WHERE id = ?");
    query.addBindValue(available);
    query.addBindValue(productId);
    return query.exec();
}

QVector<QVector<QVariant>> DatabaseManager::getProductsByShop(int shopId) {
    QVector<QVector<QVariant>> products;
    QSqlQuery query;
    query.prepare("SELECT id, name, price, category, available FROM products WHERE shop_id = ? AND available = 1");
    query.addBindValue(shopId);

    if (query.exec()) {
        while (query.next()) {
            QVector<QVariant> product;
            for (int i = 0; i < 5; ++i) {
                product.append(query.value(i));
            }
            products.append(product);
        }
    }
    return products;
}

QVector<QVector<QVariant>> DatabaseManager::getAllAvailableProducts() {
    QVector<QVector<QVariant>> products;
    QSqlQuery query("SELECT p.id, p.name, s.shop_name, p.price, p.category, p.available, s.id "
                    "FROM products p "
                    "JOIN shops s ON p.shop_id = s.id "
                    "WHERE p.available = 1");

    if (query.exec()) {
        while (query.next()) {
            QVector<QVariant> product;
            for (int i = 0; i < 7; ++i) {
                product.append(query.value(i));
            }
            products.append(product);
        }
    }
    return products;
}

int DatabaseManager::createOrder(int studentId, int shopId, double totalAmount) {
    QSqlQuery query;
    query.prepare("INSERT INTO orders (student_id, shop_id, total_amount) VALUES (?, ?, ?)");
    query.addBindValue(studentId);
    query.addBindValue(shopId);
    query.addBindValue(totalAmount);

    if (query.exec()) {
        return query.lastInsertId().toInt();
    }
    return -1;
}

bool DatabaseManager::addOrderItem(int orderId, int productId, int quantity, double price) {
    QSqlQuery query;
    query.prepare("INSERT INTO order_items (order_id, product_id, quantity, price) VALUES (?, ?, ?, ?)");
    query.addBindValue(orderId);
    query.addBindValue(productId);
    query.addBindValue(quantity);
    query.addBindValue(price);
    return query.exec();
}

bool DatabaseManager::updateOrderStatus(int orderId, const QString &status) {
    QSqlQuery query;
    query.prepare("UPDATE orders SET status = ? WHERE id = ?");
    query.addBindValue(status);
    query.addBindValue(orderId);
    return query.exec();
}

QVector<QVector<QVariant>> DatabaseManager::getOrdersByStudent(int studentId) {
    QVector<QVector<QVariant>> orders;
    QSqlQuery query;
    query.prepare("SELECT o.id, s.shop_name, o.total_amount, o.status, o.order_date "
                  "FROM orders o "
                  "JOIN shops s ON o.shop_id = s.id "
                  "WHERE o.student_id = ? "
                  "ORDER BY o.order_date DESC");
    query.addBindValue(studentId);

    if (query.exec()) {
        while (query.next()) {
            QVector<QVariant> order;
            for (int i = 0; i < 5; ++i) {
                order.append(query.value(i));
            }
            orders.append(order);
        }
    }
    return orders;
}

QVector<QVector<QVariant>> DatabaseManager::getOrdersByShop(int shopId) {
    QVector<QVector<QVariant>> orders;
    QSqlQuery query;
    query.prepare("SELECT o.id, u.username, o.total_amount, o.status, o.order_date, "
                  "(SELECT GROUP_CONCAT(p.name || ' x ' || oi.quantity) "
                  "FROM order_items oi "
                  "JOIN products p ON oi.product_id = p.id "
                  "WHERE oi.order_id = o.id) as items "
                  "FROM orders o "
                  "JOIN users u ON o.student_id = u.id "
                  "WHERE o.shop_id = ? "
                  "ORDER BY o.order_date DESC");
    query.addBindValue(shopId);

    if (query.exec()) {
        while (query.next()) {
            QVector<QVariant> order;
            for (int i = 0; i < 6; ++i) {
                order.append(query.value(i));
            }
            orders.append(order);
        }
    }
    return orders;
}

QVector<QVector<QVariant>> DatabaseManager::getOrderItems(int orderId) {
    QVector<QVector<QVariant>> items;
    QSqlQuery query;
    query.prepare("SELECT p.name, oi.quantity, oi.price "
                  "FROM order_items oi "
                  "JOIN products p ON oi.product_id = p.id "
                  "WHERE oi.order_id = ?");
    query.addBindValue(orderId);

    if (query.exec()) {
        while (query.next()) {
            QVector<QVariant> item;
            for (int i = 0; i < 3; ++i) {
                item.append(query.value(i));
            }
            items.append(item);
        }
    }
    return items;
}

double DatabaseManager::getTotalRevenue(int shopId) {
    QSqlQuery query;
    query.prepare("SELECT SUM(total_amount) FROM orders WHERE shop_id = ? AND status = 'completed'");
    query.addBindValue(shopId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double DatabaseManager::getTodayRevenue(int shopId) {
    QSqlQuery query;
    query.prepare("SELECT SUM(total_amount) FROM orders WHERE shop_id = ? AND status = 'completed' AND DATE(order_date) = DATE('now')");
    query.addBindValue(shopId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

int DatabaseManager::getTotalOrdersCount(int shopId) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM orders WHERE shop_id = ?");
    query.addBindValue(shopId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int DatabaseManager::getCompletedOrdersCount(int shopId) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM orders WHERE shop_id = ? AND status = 'completed'");
    query.addBindValue(shopId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}
