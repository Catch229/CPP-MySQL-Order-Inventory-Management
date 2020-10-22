// PPL_Final_CPP.cpp : This file contains the 'main' function. Program execution begins and ends there.
// MySQL Database interface for inventory and order management system

#include "pch.h"
#include <iostream>
#include <mysql.h>
#include <vector>
#include <sstream>

using namespace std;

MYSQL* conn = mysql_init(0);
MYSQL_ROW row;
MYSQL_RES *res;
MYSQL_STMT *stmt;

int qstate;

MYSQL_RES* dbQuery(string query) {
	const char* q = query.c_str();
	qstate = mysql_query(conn, q);
	return mysql_store_result(conn);
}

MYSQL_RES* printDBTable(string table) {
	int numCol = 0;
	printf("\n%s:\n", table);
	
	res = dbQuery("SELECT * FROM INFORMATION_SCHEMA.COLUMNS where TABLE_NAME = \"" + table + "\"");

	while (row = mysql_fetch_row(res)) {
		printf("%-20s", row[3] + string(":"));
		numCol++;
	}
	cout << endl;

	res = dbQuery("SELECT * FROM " + table);
	MYSQL_RES *resTemp = res;

	while (row = mysql_fetch_row(res)) {
		for (int i = 0; i < numCol; i++) {
			printf("%-20s", row[i]);
		}
		cout << endl;
	}

	return resTemp;

}

void createSale(string id, string item, string qty, string totalCost) {
	dbQuery("INSERT INTO sales(customer_id, item_id, amount, cost) "
			"VALUES('" + id + "', '" + item + "', '" + qty + "', '" + totalCost + "')");
}


int main()
{
	conn = mysql_real_connect(conn, "localhost", "root", "", "pplfinal", 3306, NULL, 0);
	
	int account = 0;
	int item = 0;
	int itemQty = 0;
	vector<string> invoice;

	string runAgain = "y";
	while (runAgain.compare("y") == 0) {
		invoice.clear();
		int totalCost = 0;
		printf("*******************************************\n");
		printf("*     Inventory and Order Management      *\n");
		printf("*******************************************\n");

		printDBTable("customers");
		bool account_entered = false;
		while (!account_entered) {
			printf("\nEnter customer ID or \"0\" to add a new record: ");
			cin >> ws;
			cin >> account;
			if (account == 0) {
				string name, email, address;
				printf("\nCustomer Name: ");
				cin >> ws;
				getline(cin, name);
				printf("\nCustomer Email: ");
				cin >> ws;
				getline(cin, email);
				printf("\nCustomer Address: ");
				cin >> ws;
				getline(cin, address);
				dbQuery("INSERT INTO customers(name, email, address) VALUES ('" + name + "', '" + email + "', '" + address + "')");
				account_entered = true;
				res = dbQuery("SELECT COUNT(*) FROM customers");
				row = mysql_fetch_row(res);
				account = atoi(row[0]);
			} else {
				res = dbQuery("SELECT * FROM customers");
				while (row = mysql_fetch_row(res)) {
					if (atoi(row[0]) == account) {
						account_entered = true;
						break;
					}
				}
				if (!account_entered) {
					printf("\nInvalid Customer ID.");
				}
			}
		}
		string again = "y";
		while (again.compare("y") == 0) {
			printDBTable("inventory");

			bool item_entered = false;
			while (!item_entered) {
				printf("\nEnter an item ID: ");
				cin >> ws;
				cin >> item;
				res = dbQuery("SELECT * FROM inventory");
				while (row = mysql_fetch_row(res)) {
					if (atoi(row[0]) == item) {
						item_entered = true;
						break;
					}
				}
				if (!item_entered) {
					printf("\nInvalid item ID.");
				}
			}

			bool qty_entered = false;
			while (!qty_entered) {
				printf("\nEnter item quantity: ");
				cin >> ws;
				cin >> itemQty;
				res = dbQuery("SELECT * FROM inventory WHERE id = " + to_string(item));
				while (row = mysql_fetch_row(res)) {
					if (atoi(row[2]) >= itemQty) {
						qty_entered = true;
						totalCost += atoi(row[3]) * itemQty;
						dbQuery("UPDATE inventory SET stock = " + to_string(atoi(row[2]) - itemQty) + " WHERE id = " + to_string(item));
						invoice.push_back(string(row[1]) + "\tQty: " + to_string(itemQty) + "\tCost: " + to_string(atoi(row[3]) * itemQty));
						createSale(to_string(account), to_string(item), to_string(itemQty), to_string(atoi(row[3])*itemQty));
						break;
					}
				}
				if (!qty_entered) {
					printf("Not enough stock.");
				}
			}
			printf("\nContinue ordering? (y/n): ");
			cin >> ws;
			getline(cin, again);
		}

		printf("\n\nCustomer Invoice:\n");
		for (int i = 0; i < invoice.size(); i++) {
			cout << invoice.at(i) << endl;
		}
		printf("\nTotal Cost: %.2d\n", totalCost);
		printf("Continue creating orders? (y/n): ");
		cin >> ws;
		getline(cin, runAgain);

	}
	//delete conn;
}

