#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

#include <mongocxx/exception/query_exception.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>

#include <memory>
#include <functional>

#include "message.h"
#include "account.h"

class DatabaseManager
{
private:
    std::unique_ptr<mongocxx::instance> _instance;
    mongocxx::database db;
    mongocxx::client client;
    mongocxx::uri uri;
    
public:
//    explicit DatabaseManager(QObject *parent = nullptr);
    DatabaseManager();
    /*** USER ****/
    bool registerUser(QString _id, QString password);
    bool deleteUser  (QString _id);
    bool checkUserPsw(QString _id, QString password);
    bool changePassword(QString _id, QString password);
    Account getAccount(int siteId);
    /************/

    bool insertInDB(Message mes);
    void deleteFromDB(Message mes);
    void updateDB(Message m);
    void createFile(QString nome, int user);
    QList<Symbol> retrieveFile(QString documentName);

    ~DatabaseManager();
};

#endif // DATABASEMANAGER_H
