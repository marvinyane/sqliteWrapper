#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <string>
#include <vector>
#include <memory>

#include "database_data.h"

struct sqlite3;
struct sqlite3_stmt;

namespace sql {

enum {
    DB_OK = 0,
    DB_ERROR
};

class database 
{
    public:
        database(const std::string& path, const void* pKey = NULL, int nKey = 0);
        virtual ~database();

        int exec(const std::string& sql);


        /*Cursor* should replace to shared pointer!*/

        DBDataTable* rawQuery(const std::string& sql, const std::vector<std::string>& args);

        DBDataTable* query(const std::string& table, const std::vector<std::string>& columns,
                        const std::string& where, const std::vector<std::string>& whereArgs,
                        const std::string& orderBy);

        DBDataTable* query(const std::string& table, const std::vector<std::string>& columns,
                       const std::string& where, const std::vector<std::string>& whereArgs,
                       const std::string& groupBy,const std::string& having, const std::string& orderBy);

        DBDataTable* query(const std::string& table, const std::vector<std::string>& columns,
                       const std::string& where, const std::vector<std::string>& whereArgs,
                       const std::string& groupBy,
                       const std::string& having, const std::string& orderBy, const std::string& limit);

        DBDataTable* query(bool distinct, const std::string& table, const std::vector<std::string>& columns,
                       const std::string& where, const std::vector<std::string>& whereArgs,
                       const std::string& groupBy,
                       const std::string& having, const std::string& orderBy, const std::string& limit);

        int insert(const std::string& table, const DBDataRow& values);

        int update(const std::string& table, const DBDataRow& values,
                   const std::string& where, const std::vector<std::string>& whereArgs);

        int remove(const std::string& table, const std::string& where, const std::vector<std::string>& whereArgs);

        void close();
        bool isOpen();
        bool isReadOnly();
        int getVersion();
        void setVersion(int version);
        std::string getPath();


    private:
        database(const database&);
        database& operator= (const database&);

        std::string m_path;
        sqlite3*    m_dbHandle;

        int fillTable(sqlite3_stmt* stmt, DBDataTable* dataTable);
};


}

#endif
