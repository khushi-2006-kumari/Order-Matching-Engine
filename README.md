ORDER MATCHING ENGINE:

A C++ simulation of a stock exchange's core matching logic — handles buy/sell order execution using price-time priority, similar to how real trading systems process orders.

#Features:

-> Price-time priority matching — orders are matched by best price first, then by arrival time for orders at the same price
-> Partial fills — large orders can be matched against multiple smaller orders across several trades
-> Order cancellation — O(1) lookup and removal of resting orders using a hash map
-> Real-time order book printing — view the current state of both buy and sell books at any point
