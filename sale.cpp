#include <stdio.h>
#include <time.h>

/* ========== 1. 定义商品结构体 ========== */
typedef struct {
    int id;             // 商品编号
    char name[50];      // 商品名称
    float price;        // 商品单价
    int stock;          // 商品库存
} Product;

/* ========== 2. 定义购物车项结构体 ========== */
typedef struct {
    Product product;    // 商品信息
    int quantity;       // 购买数量
    float subtotal;     // 小计金额 = price * quantity
} CartItem;

/* ========== 3. 定义购物车结构体 ========== */
typedef struct {
    CartItem items[20]; // 购物车最多放20种商品
    int item_count;     // 当前购物车中商品种类数
    float total_price;  // 购物车总金额
} ShoppingCart;

/* ========== 4. 定义交易结构体 ========== */
typedef struct {
    int transaction_id;     // 交易编号
    ShoppingCart cart;      // 本次交易的购物车
    time_t create_time;     // 交易创建时间
    int is_paid;            // 是否已支付：0未付，1已付
    float amount_paid;      // 实际支付金额
    float change;           // 找零金额
} Transaction;

/* ========== 5. 全局数据：商品库存 ========== */
Product product_database[] = {
    {1001, "Coca-Cola", 3.50, 10},
    {1002, "Pepsi-Cola", 3.50, 8},
    {1003, "Lay's Chips", 6.00, 15},
    {1004, "Oreo Cookies", 8.50, 5},
    {1005, "Mineral Water", 2.00, 20},
    {1006, "Chocolate Bar", 5.50, 12}
};
int product_count = 6;  // 商品种类总数

/* ========== 6. 辅助函数：按ID查找商品 ========== */
Product* find_product_by_id(int product_id) {
    for (int i = 0; i < product_count; i++) {
        if (product_database[i].id == product_id) {
            return &product_database[i];  // 返回商品地址
        }
    }
    return NULL;  // 没找到
}

/* ========== 7. 辅助函数：显示所有商品 ========== */
void display_all_products() {
    printf("\n========== Product List ==========\n");
    printf("%-6s %-20s %-8s %-6s\n", "ID", "Product Name", "Price", "Stock");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < product_count; i++) {
        printf("%-6d %-20s $%-7.2f %-6d\n",
               product_database[i].id,
               product_database[i].name,
               product_database[i].price,
               product_database[i].stock);
    }
    printf("========================================\n");
}

/* ========== 8. 核心函数：初始化购物车 ========== */
ShoppingCart init_shopping_cart() {
    ShoppingCart cart;
    cart.item_count = 0;    // 商品种类数从0开始
    cart.total_price = 0.0; // 总金额初始为0
    return cart;
}

/* ========== 9. 核心函数：添加商品到购物车 ========== */
int add_to_cart(ShoppingCart* cart, int product_id, int quantity) {
    // 1. 检查数量是否有效
    if (quantity <= 0) {
        printf("Error: Purchase quantity must be greater than 0!\n");
        return 0;
    }
    
    // 2. 查找商品是否存在
    Product* product = find_product_by_id(product_id);
    if (product == NULL) {
        printf("Error: Product ID %d does not exist!\n", product_id);
        return 0;
    }
    
    // 3. 检查库存是否足够
    if (product->stock < quantity) {
        printf("Error: Insufficient stock for %s! Current stock: %d\n", 
               product->name, product->stock);
        return 0;
    }
    
    // 4. 检查购物车是否已满
    if (cart->item_count >= 20) {
        printf("Error: Shopping cart is full!\n");
        return 0;
    }
    
    // 5. 检查购物车是否已有此商品
    for (int i = 0; i < cart->item_count; i++) {
        if (cart->items[i].product.id == product_id) {
            // 已有此商品，增加数量
            cart->items[i].quantity += quantity;
            cart->items[i].subtotal = cart->items[i].product.price * cart->items[i].quantity;
            
            // 更新购物车总价
            cart->total_price += product->price * quantity;
            
            printf("Updated: %s quantity increased to %d\n", product->name, cart->items[i].quantity);
            return 1;
        }
    }
    
    // 6. 购物车没有此商品，添加新项
    CartItem new_item;
    new_item.product = *product;        // 复制商品信息
    new_item.quantity = quantity;       // 设置数量
    new_item.subtotal = product->price * quantity;  // 计算小计
    
    // 将新商品添加到购物车数组
    cart->items[cart->item_count] = new_item;
    cart->item_count++;  // 商品种类数加1
    cart->total_price += new_item.subtotal;  // 更新总价
    
    printf("Added: %s x%d ($%.2f)\n", 
           product->name, quantity, new_item.subtotal);
    return 1;
}

/* ========== 10. 核心函数：从购物车移除商品 ========== */
int remove_from_cart(ShoppingCart* cart, int product_id) {
    int found_index = -1;
    
    // 1. 查找商品在购物车中的位置
    for (int i = 0; i < cart->item_count; i++) {
        if (cart->items[i].product.id == product_id) {
            found_index = i;
            break;
        }
    }
    
    // 2. 如果没找到，返回错误
    if (found_index == -1) {
        printf("Error: Product ID %d not found in cart!\n", product_id);
        return 0;
    }
    
    // 3. 从总价中减去这件商品的小计
    cart->total_price -= cart->items[found_index].subtotal;
    
    // 4. 将后面的商品向前移动一位（覆盖要删除的商品）
    for (int i = found_index; i < cart->item_count - 1; i++) {
        cart->items[i] = cart->items[i + 1];
    }
    
    // 5. 商品种类数减1
    cart->item_count--;
    
    printf("Removed product ID %d from cart\n", product_id);
    return 1;
}

/* ========== 11. 核心函数：显示购物车内容 ========== */
void display_cart(ShoppingCart* cart) {
    if (cart->item_count == 0) {
        printf("\nShopping cart is empty\n");
        return;
    }
    
    printf("\n========== Shopping cart contents ==========\n");
    printf("%-6s %-20s %-8s %-6s %-10s\n", 
           "ID", "Product Name", "Unit Price", "Quantity", "Subtotal");
    printf("------------------------------------------------\n");
    
    for (int i = 0; i < cart->item_count; i++) {
        printf("%-6d %-20s $%-7.2f %-6d $%-9.2f\n",
               cart->items[i].product.id,
               cart->items[i].product.name,
               cart->items[i].product.price,
               cart->items[i].quantity,
               cart->items[i].subtotal);
    }
    
    printf("------------------------------------------------\n");
    printf("Product Categories: %d items\n", cart->item_count);
    printf("Shopping Cart Total: $%.2f\n", cart->total_price);
    printf("================================\n");
}

/* ========== 12. 核心函数：创建交易 ========== */
Transaction create_transaction(ShoppingCart* cart) {
    Transaction trans;
    
    // 生成交易ID（简单使用时间戳）
    trans.transaction_id = time(NULL) % 100000;
    
    // 复制购物车内容
    trans.cart = *cart;
    
    // 设置交易时间
    trans.create_time = time(NULL);
    
    // 初始化支付状态
    trans.is_paid = 0;      // 未支付
    trans.amount_paid = 0.0;
    trans.change = 0.0;
    
    printf("Transaction #%d created\n", trans.transaction_id);
    return trans;
}

/* ========== 13. 核心函数：处理支付 ========== */
int process_payment(Transaction* trans, float amount_paid) {
    // 1. 检查交易是否已支付
    if (trans->is_paid) {
        printf("Error: Transaction #%d has already been paid!\n", trans->transaction_id);
        return 0;
    }
    
    // 2. 检查购物车是否为空
    if (trans->cart.item_count == 0) {
        printf("Error: Shopping cart is empty, cannot process payment!\n");
        return 0;
    }
    
    // 3. 检查支付金额是否足够
    if (amount_paid < trans->cart.total_price) {
        printf("Error: Insufficient payment amount! Required: $%.2f, Paid: $%.2f\n",
               trans->cart.total_price, amount_paid);
        return 0;
    }
    
    // 4. 计算找零
    trans->change = amount_paid - trans->cart.total_price;
    trans->amount_paid = amount_paid;
    trans->is_paid = 1;  // 标记为已支付
    
    printf("\nPayment Successful!\n");
    printf("Amount Due: $%.2f\n", trans->cart.total_price);
    printf("Amount Paid: $%.2f\n", amount_paid);
    printf("Change: $%.2f\n", trans->change);
    
    return 1;
}

/* ========== 14. 核心函数：完成交易 ========== */
int complete_transaction(Transaction* trans) {
    // 1. 检查是否已支付
    if (!trans->is_paid) {
        printf("Error: Transaction #%d is unpaid, cannot complete!\n", trans->transaction_id);
        return 0;
    }
    
    // 2. 更新商品库存
    printf("\nUpdating inventory...\n");
    for (int i = 0; i < trans->cart.item_count; i++) {
        int product_id = trans->cart.items[i].product.id;
        int quantity_sold = trans->cart.items[i].quantity;
        
        // 查找商品并减少库存
        Product* product = find_product_by_id(product_id);
        if (product != NULL) {
            product->stock -= quantity_sold;
            printf("  %s inventory updated: %d → %d\n", 
                   product->name, 
                   product->stock + quantity_sold, 
                   product->stock);
        }
    }
    
    // 3. 打印收据
    printf("\n========== Transaction completed ==========\n");
    printf("Transaction ID: #%d\n", trans->transaction_id);
    printf("Completion Time: %s", ctime(&trans->create_time));
    printf("Total Items: %d\n", trans->cart.item_count);
    printf("Total Amount: $%.2f\n", trans->cart.total_price);
    printf("Amount Paid: $%.2f\n", trans->amount_paid);
    printf("Change: $%.2f\n", trans->change);
    printf("==============================\n\n");
    
    return 1;
}

/* ========== 15. 模拟完整销售流程 ========== */
void simulate_sales_process() {
    printf("=== Unmanned Supermarket Sales Simulation ===\n\n");
    
    // 步骤1：显示所有商品
    display_all_products();
    
    // 步骤2：初始化购物车
    printf("\n1. Initializing shopping cart...");
    ShoppingCart cart = init_shopping_cart();
    printf("Completed\n");
    
    // 步骤3：模拟顾客添加商品
    printf("\n2. Simulating shopping...\n");
    add_to_cart(&cart, 1001, 2);  // Coca-Cola x2
    add_to_cart(&cart, 1003, 1);  // Lay's Chips x1
    add_to_cart(&cart, 1004, 1);  // Oreo Cookies x1
    add_to_cart(&cart, 1001, 1);  // 再买1个Coca-Cola

    // 步骤4：显示购物车
    display_cart(&cart);
    
    // 步骤5：创建交易
    printf("\n3. Creating transaction...\n");
    Transaction trans = create_transaction(&cart);
    
    // 步骤6：模拟支付
    printf("\n4. Simulating payment...\n");
    float should_pay = cart.total_price;
    float customer_pays = 30.00;  // 顾客支付30美元
    
    if (process_payment(&trans, customer_pays)) {
        // 步骤7：完成交易
        complete_transaction(&trans);
    }
    
    // 步骤8：显示更新后的商品库存
    printf("\n5. Updated product inventory:\n");
    display_all_products();
}