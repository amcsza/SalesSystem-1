#include "database.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
sqlite3 *db;
char err_msg[512];
bool init_db();
int add_product();
int query_products();
int update_stock();
int buy_product();

bool init_db(){
    int rc = sqlite3_open("sales.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return false;
    }
    char *err_msg = 0;
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
const char *sql_create_products = 
"CREATE TABLE IF NOT EXISTS products ("
"id INTEGER PRIMARY KEY AUTOINCREMENT,"
"name TEXT NOT NULL,"
"price REAL NOT NULL,"
"stock INTEGER NOT NULL"
");";
const char *sql_create_transactions = 
"CREATE TABLE IF NOT EXISTS transactions ("
"transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"create_time INTEGER NOT NULL,"
"is_paid INTEGER NOT NULL CHECK(is_paid IN (0,1)),"
"total_price REAL NOT NULL,"
"amount_paid REAL NOT NULL,"
"change REAL NOT NULL"
");";
const char *sql_create_cart_items = 
"CREATE TABLE IF NOT EXISTS cart_items ("
"item_id INTEGER PRIMARY KEY AUTOINCREMENT,"
"transaction_id INTEGER NOT NULL,"
"product_id INTEGER NOT NULL,"
"quantity INTEGER NOT NULL CHECK(quantity > 0),"
"subtotal REAL NOT NULL,"
"FOREIGN KEY(transaction_id) REFERENCES transactions(transaction_id),"
"FOREIGN KEY(product_id) REFERENCES products(id)"
");";
    rc = sqlite3_exec(db, sql_create_products, 0, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    rc = sqlite3_exec(db, sql_create_transactions, 0, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    rc = sqlite3_exec(db, sql_create_cart_items, 0, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    return true;       
}


 int add_product(const char* name, double price, int stock) {
     if(db == NULL){
        fprintf(stderr, "数据库未连接\n");
        return -1;
     }
     const char *sql = "INSERT INTO products (name, price, stock) VALUES (?, ?, ?);";
     sqlite3_stmt *stmt;
     char *err_msg = 0;
     int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
     if (rc != SQLITE_OK) {
         fprintf(stderr, "预编译SQL失败: %s\n", sqlite3_errmsg(db));
         return -1;
     }
     
        sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 2, price);
        sqlite3_bind_int(stmt, 3, stock);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "插入商品失败: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
        sqlite3_finalize(stmt);
        printf("商品%s添加成功: \n", name);
        return 0;
    }
    int query_products() {
        if(db == NULL){
            fprintf(stderr, "数据库未连接\n");
            return -1;
        }
        const char *sql = "SELECT id, name, price, stock FROM products;";
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "预编译SQL失败: %s\n", sqlite3_errmsg(db));
            return -1;
        }
        printf("商品列表:\n");
        printf("ID\t名称\t价格\t库存\n");
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char *name = sqlite3_column_text(stmt, 1);
            double price = sqlite3_column_double(stmt, 2);
            int stock = sqlite3_column_int(stmt, 3);
            printf("%d\t%s\t%.2f\t%d\n", id, name, price, stock);
        }
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "查询商品失败: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
        printf("商品查询完成\n");
        sqlite3_finalize(stmt);
        return 0;
    }
int update_stock(int id, int new_stock) {
    if(db == NULL){
        fprintf(stderr, "数据库未连接\n");
        return -1;
    }
    const char *sql = "UPDATE products SET stock = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "预编译SQL失败: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_int(stmt, 1, new_stock);
    sqlite3_bind_int(stmt, 2, id);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "更新库存失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    printf("商品ID %d 库存更新为 %d 成功\n", id, new_stock);
    return 0;
}
int buy_product(int id, int buy_num , double amount_paid, double* change) {
    if(db == NULL){
        fprintf(stderr, "数据库未连接\n");
        return -1;
    }
    const char *query_sql = "SELECT price, stock FROM products WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "预编译SQL失败: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        fprintf(stderr, "商品不存在或库存不足\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    double price = sqlite3_column_double(stmt, 0);
    int stock = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);
    if (stock < buy_num) {
        fprintf(stderr, "库存不足，当前库存: %d\n", stock);
        return -1;
    }
    double total_price = price * buy_num;
    if (amount_paid < total_price) {
        fprintf(stderr, "支付金额不足，需支付: %.2f\n", total_price);
        return -1;
    }
    *change = amount_paid - total_price;
    const char *update_sql = "UPDATE products SET stock = stock - ? WHERE id = ?;";
    rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "预编译SQL失败: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    sqlite3_bind_int(stmt, 1, buy_num);
    sqlite3_bind_int(stmt, 2, id);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "更新库存失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    printf("购买成功！总价: %.2f找零: %.2f\n", total_price, *change);
    return 0;
}
    

