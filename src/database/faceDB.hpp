#pragma once
#include <iostream>
#include <sqlite3.h>
using namespace std;

class faceDB {
private:
    sqlite3* db;  
    std::string dbName;  
public:
    faceDB(const std::string& dbName);  // Constructor
    ~faceDB();  // Destructor
    bool openDB();  // Open the database
    bool closeDB();  // Close the database
    bool createTable();  // Create a table in the database
    bool insertData(const std::string& name, const std::string& imagePath);  // Insert data into the table
    bool deleteData(const std::string& name);  // Delete data from the table
    bool searchData(const std::string& name);  // Search data from the table
};