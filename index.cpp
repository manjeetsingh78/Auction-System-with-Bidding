#include <iostream>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace chrono;

class User;
class Auction;
class AuctionSystem;

struct Bid {
    string userId;
    double amount;
    time_point<steady_clock> timestamp;
    string itemId;
    
    Bid(const string& uid, double amt, const string& iid)
        : userId(uid), amount(amt), timestamp(steady_clock::now()), itemId(iid) {}
    
    // For priority queue (max heap based on amount)
    bool operator<(const Bid& other) const {
        if (amount != other.amount) {
            return amount < other.amount; // Higher amount has higher priority
        }
        return timestamp > other.timestamp; // Earlier timestamp has higher priority for same amount
    }
};

    void displayAuctionDetails(const string& itemId) {
        if (auctions.find(itemId) == auctions.end()) {
            cout << "Auction not found!" << endl;
            return;
        }
        
        auctions[itemId].displayAuctionInfo();
    }


    void displayUserProfile() const {
        if (currentUserId.empty()) {
            cout << "Please login first!" << endl;
            return;
        }
        
        const User& user = users.at(currentUserId);
        cout << "\n=== User Profile ===" << endl;
        cout << "Username: " << user.username << endl;
        cout << "Email: " << user.email << endl;
        cout << "Balance: $" << user.balance << endl;
        cout << "Bids Placed: " << user.bidHistory.size() << endl;
        cout << "Items Owned: " << user.ownedItems.size() << endl;
        cout << "Items Sold: " << user.soldItems.size() << endl;
        
        if (!userAuctions.empty() && userAuctions.find(currentUserId) != userAuctions.end()) {
            cout << "Auctions Created: " << userAuctions.at(currentUserId).size() << endl;
        }
    }


    void displayBidHistory(const string& itemId) {
        if (auctions.find(itemId) == auctions.end()) {
            cout << "Auction not found!" << endl;
            return;
        }
        
        auto bidHistory = auctions[itemId].getBidHistory();
        cout << "\n=== Bid History for " << itemId << " ===" << endl;
        
        if (bidHistory.empty()) {
            cout << "No bids placed yet." << endl;
            return;
        }
        
        sort(bidHistory.begin(), bidHistory.end(), [](const Bid& a, const Bid& b) {
            return a.amount > b.amount; // Sort by amount descending
        });
        
        int rank = 1;
        for (const auto& bid : bidHistory) {
            cout << rank++ << ". User: " << bid.userId 
                 << " | Amount: $" << bid.amount 
                 << " | Time: " << duration_cast<seconds>(bid.timestamp.time_since_epoch()).count() % 10000 << endl;
        }
    }


    void endAuction(const string& itemId) {
        if (auctions.find(itemId) == auctions.end()) {
            cout << "Auction not found!" << endl;
            return;
        }
        
        Auction& auction = auctions[itemId];
        
        if (!auction.isActive()) {
            cout << "Auction already ended!" << endl;
            return;
        }
        
        auction.endAuction();
        Bid highestBid = auction.getHighestBid();
        
        cout << "\n=== Auction Ended ===" << endl;
        
        if (highestBid.userId.empty()) {
            cout << "No bids were placed. Item remains unsold." << endl;
        } else if (!auction.hasReserveBeenMet()) {
            cout << "Reserve price not met. Item remains unsold." << endl;
            cout << "Highest bid: $" << highestBid.amount << " by " << highestBid.userId << endl;
        } else {
            cout << "Item sold to " << highestBid.userId << " for $" << highestBid.amount << endl;
            
            // Update user records
            users[highestBid.userId].deductBalance(highestBid.amount);
            users[highestBid.userId].addOwnedItem(itemId);
            
            const auto& item = auction.getItem();
            users[item.sellerId].addBalance(highestBid.amount);
            users[item.sellerId].addSoldItem(itemId);
        }
    }



    void addBalance(double amount) {
        if (currentUserId.empty()) {
            cout << "Please login first!" << endl;
            return;
        }
        
        users[currentUserId].addBalance(amount);
        cout << "Balance added successfully! New balance: $" << users[currentUserId].balance << endl;
    }

    
    void searchAuctions(const string& keyword) const {
        cout << "\n=== Search Results for: " << keyword << " ===" << endl;
        bool found = false;
        
        for (const auto& pair : auctions) {
            const auto& item = pair.second.getItem();
            if (item.name.find(keyword) != string::npos || item.description.find(keyword) != string::npos) {
                found = true;
                cout << "ID: " << item.id << " | " << item.name << " | Current Price: $" << pair.second.getCurrentPrice()<< " | Status: " << (pair.second.isActive() ? "Active" : "Ended") << endl;
            }
        }
        
        if (!found) {
            cout << "No auctions found matching: " << keyword << endl;
        }
    }

    void displayTopBidders(const string& itemId) const {
        if (auctions.find(itemId) == auctions.end()) {
            cout << "Auction not found!" << endl;
            return;
        }
        
        auto userBids = auctions.at(itemId).getUserBids();
        vector<pair<string, double>> bidders;
        
        for (const auto& pair : userBids) {
            bidders.push_back({pair.first, pair.second});
        }
        
        sort(bidders.begin(), bidders.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
            return a.second > b.second;
        });
        
        cout << "\n=== Top Bidders for " << itemId << " ===" << endl;
        int rank = 1;
        for (const auto& bidder : bidders) {
            cout << rank++ << ". " << bidder.first << " - $" << bidder.second << endl;
            if (rank > 5) break; // Show top 5 only
        }
    }


    void displayMenu() const {
        cout << "\n=== Auction System Menu ===" << endl;
        cout << "1. Register User" << endl;
        cout << "2. Login" << endl;
        cout << "3. Logout" << endl;
        cout << "4. Create Auction" << endl;
        cout << "5. Place Bid" << endl;
        cout << "6. View Active Auctions" << endl;
        cout << "7. View Auction Details" << endl;
        cout << "8. View User Profile" << endl;
        cout << "9. View Bid History" << endl;
        cout << "10. End Auction" << endl;
        cout << "11. Add Balance" << endl;
        cout << "12. Search Auctions" << endl;
        cout << "13. View Top Bidders" << endl;
        cout << "0. Exit" << endl;
        cout << "Choice: ";
    }



    void run() {
        int choice;
        string input, username, email, itemName, description, itemId, keyword;
        double startingPrice, reservePrice, amount;
        int duration;
        
        cout << "Welcome to Advanced Auction System!" << endl;
        
        while (true) {
            displayMenu();
            cin >> choice;
            cin.ignore(); // Clear input buffer
            
            switch (choice) {
                case 1:
                    cout << "Enter username: ";
                    getline(cin, username);
                    cout << "Enter email: ";
                    getline(cin, email);
                    registerUser(username, email);
                    break;
                    
                case 2:
                    cout << "Enter username: ";
                    getline(cin, username);
                    loginUser(username);
                    break;
                    
                case 3:
                    logoutUser();
                    break;
                    
                case 4:
                    cout << "Enter item name: ";
                    getline(cin, itemName);
                    cout << "Enter description: ";
                    getline(cin, description);
                    cout << "Enter starting price: $";
                    cin >> startingPrice;
                    cout << "Enter reserve price: $";
                    cin >> reservePrice;
                    cout << "Enter duration (minutes): ";
                    cin >> duration;
                    createAuction(itemName, description, startingPrice, reservePrice, duration);
                    break;
                    
                case 5:
                    cout << "Enter item ID: ";
                    getline(cin, itemId);
                    cout << "Enter bid amount: $";
                    cin >> amount;
                    placeBid(itemId, amount);
                    break;
                    
                case 6:
                    displayActiveAuctions();
                    break;
                    
                case 7:
                    cout << "Enter item ID: ";
                    getline(cin, itemId);
                    displayAuctionDetails(itemId);
                    break;
                    
                case 8:
                    displayUserProfile();
                    break;
                    
                case 9:
                    cout << "Enter item ID: ";
                    getline(cin, itemId);
                    displayBidHistory(itemId);
                    break;
                    
                case 10:
                    cout << "Enter item ID: ";
                    getline(cin, itemId);
                    endAuction(itemId);
                    break;
                    
                case 11:
                    cout << "Enter amount to add: $";
                    cin >> amount;
                    addBalance(amount);
                    break;
                    
                case 12:
                    cout << "Enter search keyword: ";
                    getline(cin, keyword);
                    searchAuctions(keyword);
                    break;
                    
                case 13:
                    cout << "Enter item ID: ";
                    getline(cin, itemId);
                    displayTopBidders(itemId);
                    break;
                    
                case 0:
                    cout << "Thank you for using Advanced Auction System!" << endl;
                    return;
                    
                default:
                    cout << "Invalid choice! Please try again." << endl;
            }
        }
    }
};



int main() {
    AuctionSystem system;
    system.run();
    return 0;
}