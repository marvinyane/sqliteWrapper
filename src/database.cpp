#include <iterator>
#include <iostream>

#include "database.h"
#include "sqlite3.h"

namespace sql {

database::database(const std::string &path, const void *pKey, int nKey)
    : m_path(path)
{
    // create database
    sqlite3* handle = NULL;

    int err = sqlite3_open_v2(path.c_str(), &handle,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                              NULL);

    if (err != SQLITE_OK) {
        // error!
    }
    else {
        if (pKey && (nKey > 0)) {
            err = sqlite3_key(handle, pKey, nKey);
            std::cout << "sqlite key error " << err << "\n";
            if (err != SQLITE_OK) {
                sqlite3_close(handle);
                return ;
            }
        }

        m_dbHandle = handle;
    }
}

database::~database()
{
    if (m_dbHandle) {
        int err = sqlite3_close(m_dbHandle);
        if (err != SQLITE_OK) {
            // error
        }
    }
}

int database::exec(const std::string &sql)
{
    // why not exec directly?
    int err = sqlite3_exec(m_dbHandle, sql.c_str(), NULL, NULL, NULL);

    if (err != SQLITE_OK) {
        // error?
    }

    return err;
}

DBDataTable *database::rawQuery(const std::string &sql, const std::vector<std::string> &args)
{
    sqlite3_stmt *stmt = NULL;

    int err = sqlite3_prepare_v2(m_dbHandle, sql.data(), sql.length(), &stmt, NULL);
    if (err != SQLITE_OK) {
        // error?
    }

    for (int i = 0; i < args.size(); i++) {
        sqlite3_bind_text(stmt, i+1, args[i].data(), args[i].length(), SQLITE_TRANSIENT);
    }

    // replace to shared pointer? how to check result?
    DBDataTable* dataTable = new DBDataTable(0);

    int result = fillTable(stmt, dataTable);
    if (result <= 0) {
        //
        delete dataTable;
        return NULL;
    }

    return dataTable;
}

DBDataTable *database::query(const std::string &table, const std::vector<std::string> &columns,
                             const std::string &where, const std::vector<std::string> &whereArgs,
                             const std::string &orderBy)
{
    std::string groupBy;
    std::string having;
    std::string limit;

    return query(false, table, columns, where, whereArgs, groupBy, having, orderBy, limit);
}

DBDataTable *database::query(bool distinct, const std::string &table,
                             const std::vector<std::string> &columns,
                             const std::string &where, const std::vector<std::string> &whereArgs,
                             const std::string &groupBy, const std::string &having,
                             const std::string &orderBy, const std::string &limit)
{
    std::string sql("SELECT ");
    if (distinct) {
        sql.append(" DISTINCT ");
    }

    if (columns.size() > 0) {
        bool isFirst = true;
        for (auto it = columns.begin(); it != columns.end(); it++) {
            if (it->length() > 0) {
                if (isFirst) {
                    isFirst = false;
                }
                else {
                    sql.append(", ");
                }

                sql.append(*it);
            }
        }
        sql.append(" ");
    }
    else {
        sql.append(" * ");
    }

    sql.append(" FROM ");
    sql.append(table.data());
    if (where.length() > 0) {
        sql.append(" WHERE ");
        sql.append(where.data());
    }
    if (groupBy.length() > 0) {
        sql.append(" GROUP BY ");
        sql.append(groupBy.data());
    }
    if (having.length() > 0) {
        sql.append(" HAVING ");
        sql.append(having.data());
    }
    if (orderBy.length() > 0) {
        sql.append(" ORDER BY ");
        sql.append(orderBy.data());
    }
    if (limit.length() > 0) {
        sql.append(" LIMIT ");
        sql.append(limit.data());
    }

    return rawQuery(sql, whereArgs);
}

// return the row id of inserted
int database::insert(const std::string &table, const DBDataRow &values)
{
    std::string sql("INSERT");
    sql.append(" INTO ");
    sql.append(table.data());
    sql.append("(");

    //
    int length = values.getColumnCount();
    for (int i = 0; i < length; i++) {
        if (i != 0) {
            sql.append(", ");
        }
        sql.append(values.getColumnName(i));
    }

    sql.append(")");
    sql.append(" VALUES (");
    for (int i = 0; i < length; ++i) {
        if (i > 0) {
            sql.append(", ?");
        }
        else {
            sql.append("?");
        }
    }
    sql.append(")");

    sqlite3_stmt *stmt = NULL;

    int err = sqlite3_prepare_v2(m_dbHandle, sql.data(), sql.length(), &stmt, NULL);
    if (err != SQLITE_OK) {
        // error?
        return -1;
    }

    for (int i = 0; i < length; i++) {
        DBDataType type = values.type(i);
        int err = 0;
        switch (type) {
        case DBDataType_Integer:
            err = sqlite3_bind_int64(stmt, i+1, values.getLong(i));
            break;
        case DBDataType_Float:
            err = sqlite3_bind_double(stmt, i+1, values.getDouble(i));
            break;
        case DBDataType_String:
        {
            size_t len = 0;
            const char* sql = values.getString(i, len);
            err = sqlite3_bind_text(stmt, i+1, sql, len, SQLITE_TRANSIENT);
            break;
        }
        case DBDataType_Blob:
        {
            size_t len = 0;
            const void* blob = values.getBlob(i, len);
            err = sqlite3_bind_blob(stmt, i+1, blob, len, SQLITE_TRANSIENT);
            break;
        }
        case DBDataType_Null:
        default:
            break;
        }
    }

    // step!
    err = sqlite3_step(stmt);
    if (err != SQLITE_DONE) {
        // error
        return -1;
    }

    return sqlite3_last_insert_rowid(m_dbHandle);
}

// return number changes
int database::update(const std::string &table, const DBDataRow &values, const std::string &where, const std::vector<std::string> &whereArgs)
{
    std::string sql("UPDATE ");
    sql.append(table.data());
    sql.append(" SET ");

    //
    int length = values.getColumnCount();
    for (int i = 0; i < length; i++) {
        if (i != 0) {
            sql.append(", ");
        }
        sql.append(values.getColumnName(i));
        sql.append("=?");
    }

    if (where.length() > 0) {
        sql.append(" WHERE ");
        sql.append(where.data());
    }

    sqlite3_stmt *stmt = NULL;

    int err = sqlite3_prepare_v2(m_dbHandle, sql.data(), sql.length(), &stmt, NULL);
    if (err != SQLITE_OK) {
        // error?
        return -1;
    }

    for (int i = 0; i < length; i++) {
        DBDataType type = values.type(i);
        int err = 0;
        switch (type) {
        case DBDataType_Integer:
            err = sqlite3_bind_int64(stmt, i+1, values.getLong(i));
            break;
        case DBDataType_Float:
            err = sqlite3_bind_double(stmt, i+1, values.getDouble(i));
            break;
        case DBDataType_String:
        {
            size_t len = 0;
            const char* sql = values.getString(i, len);
            err = sqlite3_bind_text(stmt, i+1, sql, len, SQLITE_TRANSIENT);
            break;
        }
        case DBDataType_Blob:
        {
            size_t len = 0;
            const void* blob = values.getBlob(i, len);
            err = sqlite3_bind_blob(stmt, i+1, blob, len, SQLITE_TRANSIENT);
            break;
        }
        case DBDataType_Null:
        default:
            break;
        }
    }

    for (int i = 0; i < whereArgs.size(); i++) {
        sqlite3_bind_text(stmt, i+1+length, whereArgs[i].data(), whereArgs[i].length(), SQLITE_TRANSIENT);
    }

    err = sqlite3_step(stmt);
    if (err != SQLITE_DONE) {
        return -1;
    }

    return sqlite3_changes(m_dbHandle);
}

int database::remove(const std::string &table, const std::string &where,
                     const std::vector<std::string> &whereArgs)
{
    std::string sql("DELETE FROM ");
    sql.append(table.data());
    if (0 == where.length()) {
        sql.append("");
    }
    else {
        sql.append(" WHERE ");
        sql.append(where.data());
    }

    sqlite3_stmt *stmt = NULL;

    int err = sqlite3_prepare_v2(m_dbHandle, sql.data(), sql.length(), &stmt, NULL);
    if (err != SQLITE_OK) {
        return -1;
    }

    for (int i = 0; i < whereArgs.size(); i++) {
        sqlite3_bind_text(stmt, i+1, whereArgs[i].data(), whereArgs[i].length(), SQLITE_TRANSIENT);
    }

    err = sqlite3_step(stmt);
    if (err != SQLITE_DONE) {
        return -1;
    }

    return sqlite3_changes(m_dbHandle);
}

void database::close()
{
    sqlite3_close(m_dbHandle);
}

bool database::isOpen()
{
    return (m_dbHandle != NULL);
}

int database::fillTable(sqlite3_stmt* stmt, DBDataTable* dataTable)
{
    int numColumns = sqlite3_column_count(stmt);
    dataTable->setColumnCount(numColumns);

    int addedRows = 0;

    while (1) {
        int err = sqlite3_step(stmt);
        if (err == SQLITE_ROW) {
            dataTable->addRow();
            for (int i = 0; i < numColumns; i++) {
                int type = sqlite3_column_type(stmt, i);
                if (type == SQLITE_TEXT) {
                    // TEXT data
                    dataTable->setColumnType(i, DBDataType_String);
                    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    size_t sizeIncludingNull = sqlite3_column_bytes(stmt, i) + 1;
                    if (!dataTable->putString(addedRows, i, text, sizeIncludingNull)) {
                        // NCDBH_SQL_LOGD("Failed allocating %u bytes for text at %d,%d", sizeIncludingNull, addedRows, i);
                        std::cout << "failed allocating bytes\n";
                        break;
                    }
                    // NCDBH_SQL_LOGD("%d,%d is TEXT with %u bytes", addedRows, i, sizeIncludingNull);
                }
                else if (type == SQLITE_INTEGER) {
                    // INTEGER data
                    dataTable->setColumnType(i, DBDataType_Integer);
                    int64_t value = sqlite3_column_int64(stmt, i);
                    if (!dataTable->putLong(addedRows, i, value)) {
                        // NCDBH_SQL_LOGD("Failed allocating space for a long in column %d", i);
                        break;
                    }
                    // NCDBH_SQL_LOGD("%d,%d is INTEGER 0x%016llx", addedRows, i, value);
                }
                else if (type == SQLITE_FLOAT) {
                    // FLOAT data
                    dataTable->setColumnType(i, DBDataType_Float);
                    double value = sqlite3_column_double(stmt, i);
                    if (!dataTable->putDouble(addedRows, i, value)) {
                        // NCDBH_SQL_LOGD("Failed allocating space for a double in column %d", i);
                        break;
                    }
                    // NCDBH_SQL_LOGD("%d,%d is FLOAT %lf", addedRows, i, value);
                }
                else if (type == SQLITE_BLOB) {
                    // BLOB data
                    dataTable->setColumnType(i, DBDataType_Blob);
                    const void* blob = sqlite3_column_blob(stmt, i);
                    size_t size = sqlite3_column_bytes(stmt, i);
                    if (!dataTable->putBlob(addedRows, i, blob, size)) {
                        // NCDBH_SQL_LOGD("Failed allocating %u bytes for blob at %d,%d", size, addedRows, i);
                        break;
                    }
                    // NCDBH_SQL_LOGD("%d,%d is Blob with %u bytes", addedRows, i, size);
                }
                else if (type == SQLITE_NULL) {
                    // NULL field
                    dataTable->setColumnType(i, DBDataType_Null);
                    if (!dataTable->putNull(addedRows, i)) {
                        // NCDBH_SQL_LOGD("Failed allocating space for a null in column %d", i);
                        break;
                    }
                    // NCDBH_SQL_LOGD("%d,%d is NULL", addedRows, i);
                }
                else {
                    // Unknown data
                    // NCDBH_SQL_LOGE("Unknown column type when filling database data table");
                    std::cout << "Unknown column type when filling database data table";
                    break;
                }
            }
            addedRows++;
        }
        else if (err == SQLITE_DONE) {
            // complete!
            std::cout << "complete!\n";
            break;
        }
        else if (err == SQLITE_LOCKED || err == SQLITE_BUSY) {
            // retry?
            break;
        }
        else {
            // exception?
            std::cout << "query error with : " << err << "\n";
            break;
        }
    }

    return addedRows;
}

}



