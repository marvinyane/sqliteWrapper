#ifndef DBDATA_H
#define DBDATA_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#include <stdint.h>
#include <string>
#include <vector>
#include <cstring>

namespace sql
{
    enum DBDataType
    {
        DBDataType_Null = 0,
        DBDataType_Integer = 1,
        DBDataType_Float = 2,
        DBDataType_String = 3,
        DBDataType_Blob = 4
    };

    class DBDataCell
    {
    public:
        DBDataCell();
        DBDataCell(const std::string& name);
        DBDataCell(const DBDataCell& x);
        ~DBDataCell();

        bool operator==(const DBDataCell& x) const;
        bool operator!=(const DBDataCell& x) const;
        DBDataCell& operator=(const DBDataCell& x);

        bool putBlob(const void* value, size_t size);
        bool putString(const char* value, size_t length);
        bool putLong(int value);
        bool putDouble(double value);
        bool putNull();

        DBDataType type() const;
        int getLong() const;
        double getDouble() const;
        const char* getString(size_t& length) const;
        const void* getBlob(size_t& outSize) const;

        const std::string& getName() const
        {
            return m_name;
        }

        void setName(const char* name)
        {
            m_name.assign(name, strlen(name));
        }

    private:
        struct Cell
        {
            DBDataType type;
            union
            {
                double d;
                int l;
                struct
                {
                    void * ptr;
                    size_t size;
                } buffer;
            } data;

            Cell()
            : type(DBDataType_Null)
            {
                memset(&data, 0, sizeof(data));
            }
        } __attribute((packed));

        Cell m_cell;

        std::string m_name;
    };

    /**
     * DBDataRow
     *
     * Define a data row in DBDataTable.
     */
    class DBDataRow
    {
    public:
        DBDataRow(int columnCount);
        DBDataRow(const DBDataRow& x);
        virtual ~DBDataRow();

        bool operator==(const DBDataRow& x) const;
        DBDataRow& operator=(const DBDataRow& x);

        bool putBlob(int index, const void* value, size_t size, const char* name = NULL);
        bool putString(int index, const char* value, size_t length, const char* name = NULL);
        bool putLong(int index, int value, const char* name = NULL);
        bool putDouble(int index, double value, const char* name = NULL);
        bool putNull(int index, const char* name = NULL);

        int getColumnCount() const;
        DBDataType type(int index) const;
        int getLong(int index) const;
        double getDouble(int index) const;
        const char* getString(int index, size_t& length) const;
        const void* getBlob(int index, size_t& size) const;

        const std::string& getColumnName(int index) const;

    private:
        int m_count;
        DBDataCell* m_cells;

        DBDataRow();
    };

    /**
     * DBDataTable
     *
     * Define a data table to insert or update multi-row data in SQLite database quickly.
     */
    class DBDataTable
    {
    public:
        DBDataTable(int columnCount);
        DBDataTable(int rowCount, int columnCount);
        virtual ~DBDataTable();

        bool setRowCount(int rowCount);
        int getRowCount();
        bool setColumnCount(int columnCount);
        int getColumnCount();
        bool setColumnType(int column, DBDataType type);
        DBDataType getColumnType(int column);
        bool reset();
        void addRow();

        bool putBlob(int row, int column, const void* value, size_t size);
        bool putString(int row, int column, const char* value, size_t len);
        bool putLong(int row, int column, int value);
        bool putDouble(int row, int column, double value);
        bool putNull(int row, int column);

        DBDataType getType(int row, int column);
        int getLong(int row, int column);
        double getDouble(int row, int column);
        const char* getString(int row, int column, size_t& length);
        const void* getBlob(int row, int column, size_t& size);

    private:
        /*vector store smart pointer*/
        std::vector<DBDataRow*>  m_rowSpList;
        int    m_columnCount;
        DBDataType*     m_columnTypes;

        DBDataTable();
        DBDataTable(const DBDataTable&);
        DBDataTable& operator=(const DBDataTable&);
    };

} /* namespace nutshell */

#endif /* DBDATA_H */
/* EOF */
