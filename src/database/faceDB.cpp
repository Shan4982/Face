#include "faceDB.hpp"

faceDB::faceDB(const std::string& dbName) : dbName(dbName) {
    db = nullptr;  // Initialize db to nullptr
}

faceDB::~faceDB() {
    closeDB();  // Close the database when the object is destroyed
}

bool faceDB::openDB() {
    int rc = sqlite3_open("../../data/facedata.db", &db);
    if(rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    else {
        std::cout << "Opened database successfully" << std::endl;
        return true;
    }
}

bool faceDB::closeDB() {
    if(db) {
        int rc = sqlite3_close(db);
        return rc == SQLITE_OK;
    } else {
        return true;  // Return true if db is nullptr (already closed)}
}
cout<<"close db successfully"<<endl;
}

bool faceDB::createTable(){
    char* errMsg = 0;
    string sql = "create table if not exists facedata (name text, imagePath text,id int)";
    int rc = sqlite3_exec(db,sql.c_str(),0,0,&errMsg);
    if(rc != SQLITE_OK){
        std::cerr << "Error creating table: " << errMsg << std::endl;
        sqlite3_free(errMsg);  // Free the error message memory
        return false;
    }
}
