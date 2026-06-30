#include <bits/stdc++.h>
using namespace std;

enum class Side { BUY, SELL };

struct Order {
    int id;
    Side side;
    double price;
    int qty;
    long long timestamp;
};

class MatchingEngine {
private:
    // Buy side: highest price first -> use map sorted descending
    map<double, queue<Order>, greater<double>> buyBook;
    // Sell side: lowest price first -> map sorted ascending (default)
    map<double, queue<Order>> sellBook;

    // For cancellation: track order location (side + price) by id
    unordered_map<int, pair<Side, double>> orderLocation;

    long long timeCounter = 0;
    int tradeCount = 0;

    void logTrade(const Order &buy, const Order &sell, int qty, double price) {
        tradeCount++;
        cout << "TRADE #" << tradeCount
             << " | BuyID=" << buy.id
             << " SellID=" << sell.id
             << " | Qty=" << qty
             << " | Price=" << fixed << setprecision(2) << price
             << endl;
    }

public:
    void addOrder(int id, Side side, double price, int qty) {
        Order order{id, side, price, qty, timeCounter++};

        if (side == Side::BUY) {
            matchBuy(order);
        } else {
            matchSell(order);
        }
    }

    void matchBuy(Order order) {
        // Try to match against sell book (lowest sell price first)
        while (order.qty > 0 && !sellBook.empty()) {
            auto bestSellIt = sellBook.begin();
            double sellPrice = bestSellIt->first;

            if (sellPrice > order.price) break; // no match possible, buy price too low

            queue<Order> &q = bestSellIt->second;
            while (order.qty > 0 && !q.empty()) {
                Order &sellOrder = q.front();
                int tradedQty = min(order.qty, sellOrder.qty);

                logTrade(order, sellOrder, tradedQty, sellPrice);

                order.qty -= tradedQty;
                sellOrder.qty -= tradedQty;

                if (sellOrder.qty == 0) {
                    orderLocation.erase(sellOrder.id);
                    q.pop();
                }
            }
            if (q.empty()) sellBook.erase(bestSellIt);
        }

        // Leftover quantity rests in the buy book
        if (order.qty > 0) {
            buyBook[order.price].push(order);
            orderLocation[order.id] = {Side::BUY, order.price};
        }
    }

    void matchSell(Order order) {
        // Try to match against buy book (highest buy price first)
        while (order.qty > 0 && !buyBook.empty()) {
            auto bestBuyIt = buyBook.begin();
            double buyPrice = bestBuyIt->first;

            if (buyPrice < order.price) break; // no match possible, sell price too high

            queue<Order> &q = bestBuyIt->second;
            while (order.qty > 0 && !q.empty()) {
                Order &buyOrder = q.front();
                int tradedQty = min(order.qty, buyOrder.qty);

                logTrade(buyOrder, order, tradedQty, buyPrice);

                order.qty -= tradedQty;
                buyOrder.qty -= tradedQty;

                if (buyOrder.qty == 0) {
                    orderLocation.erase(buyOrder.id);
                    q.pop();
                }
            }
            if (q.empty()) buyBook.erase(bestBuyIt);
        }

        // Leftover quantity rests in the sell book
        if (order.qty > 0) {
            sellBook[order.price].push(order);
            orderLocation[order.id] = {Side::SELL, order.price};
        }
    }

    bool cancelOrder(int id) {
        auto it = orderLocation.find(id);
        if (it == orderLocation.end()) {
            cout << "Cancel failed: Order " << id << " not found.\n";
            return false;
        }

        Side side = it->second.first;
        double price = it->second.second;

        if (side == Side::BUY) {
            auto &q = buyBook[price];
            queue<Order> temp;
            while (!q.empty()) {
                if (q.front().id != id) temp.push(q.front());
                q.pop();
            }
            q = temp;
            if (q.empty()) buyBook.erase(price);
        } else {
            auto &q = sellBook[price];
            queue<Order> temp;
            while (!q.empty()) {
                if (q.front().id != id) temp.push(q.front());
                q.pop();
            }
            q = temp;
            if (q.empty()) sellBook.erase(price);
        }

        orderLocation.erase(it);
        cout << "Order " << id << " cancelled.\n";
        return true;
    }

    void printBook() {
    cout << "\n----- ORDER BOOK -----\n";
    cout << "SELL SIDE (low to high):\n";
    for (auto it = sellBook.begin(); it != sellBook.end(); ++it) {
        double price = it->first;
        queue<Order> temp = it->second;
        int totalQty = 0;
        while (!temp.empty()) { totalQty += temp.front().qty; temp.pop(); }
        cout << "  Price " << fixed << setprecision(2) << price
             << " | Qty " << totalQty << "\n";
    }

    cout << "BUY SIDE (high to low):\n";
    for (auto it = buyBook.begin(); it != buyBook.end(); ++it) {
        double price = it->first;
        queue<Order> temp = it->second;
        int totalQty = 0;
        while (!temp.empty()) { totalQty += temp.front().qty; temp.pop(); }
        cout << "  Price " << fixed << setprecision(2) << price
             << " | Qty " << totalQty << "\n";
    }
    cout << "-----------------------\n\n";
}
};

int main() {
    MatchingEngine engine;

    // Simulated incoming orders: (id, side, price, qty)
    engine.addOrder(1, Side::SELL, 101.5, 50);
    engine.addOrder(2, Side::SELL, 100.0, 30);
    engine.addOrder(3, Side::BUY, 99.0, 20);

    engine.printBook();

    // This buy crosses the spread -> should trigger trades
    engine.addOrder(4, Side::BUY, 101.0, 40);

    engine.printBook();

    // Cancel a resting order
    engine.cancelOrder(3);

    engine.printBook();

    // Partial fill example
    engine.addOrder(5, Side::SELL, 99.0, 60);

    engine.printBook();

    return 0;
}