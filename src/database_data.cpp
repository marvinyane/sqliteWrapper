#include <stdlib.h>
#include <cstring>
#include "database_data.h"

#define LOG_TAG "dbhelper"

#define DBDATA_CELL_FREE(X) \
    { \
        if ((DBDataType_String == X.type || DBDataType_Blob == X.type) \
                && NULL != X.data.buffer.ptr) { \
            free(X.data.buffer.ptr); \
            X.data.buffer.ptr = 0; \
            X.data.buffer.size = 0; \
        } \
    }

namespace sql
{
    DBDataCell::DBDataCell()
        :m_name()
    {
        // LOGD("DBDataCell::DBDataCell()");
        memset(&m_cell, 0x00, sizeof(Cell));
    }

    DBDataCell::DBDataCell(const std::string &name)
        :m_name(name)
    {
        memset(&m_cell, 0x00, sizeof(Cell));
    }

    DBDataCell::DBDataCell(const DBDataCell& x)
        :m_name()
    {
        // LOGD("DBDataCell::DBDataCell(const DBDataCell& x)");
        memcpy(&m_cell, &(x.m_cell), sizeof(Cell));
        if ((DBDataType_String == m_cell.type || DBDataType_Blob == m_cell.type)
            && NULL != x.m_cell.data.buffer.ptr) {
            m_cell.data.buffer.ptr = malloc(m_cell.data.buffer.size);
            // LOGD("+++malloced %p, size[%u]", m_cell.data.buffer.ptr, m_cell.data.buffer.size);
            if (NULL != m_cell.data.buffer.ptr) {
                memcpy(m_cell.data.buffer.ptr,
                        x.m_cell.data.buffer.ptr,
                        x.m_cell.data.buffer.size);
            }
            else {
                // error
                m_cell.type = DBDataType_Null;
            }
        }
    }

    DBDataCell::~DBDataCell()
    {
        // LOGD("DBDataCell::~DBDataCell()");
        DBDATA_CELL_FREE(m_cell);
    }

    bool DBDataCell::operator==(const DBDataCell& x) const
    {
        // LOGD("DBDataCell::operator==(const DBDataCell& x)");
        if (m_cell.type != x.m_cell.type) {
            return false;
        }

        if (DBDataType_String == m_cell.type
            || DBDataType_Blob == m_cell.type) {
            if (m_cell.data.buffer.size != x.m_cell.data.buffer.size) {
                return false;
            }
            if (NULL == m_cell.data.buffer.ptr
                || NULL == x.m_cell.data.buffer.ptr) {
                return false;
            }
            int r = memcmp(m_cell.data.buffer.ptr,
                    x.m_cell.data.buffer.ptr,
                    m_cell.data.buffer.size);
            if (r) {
                return false;
            }
        }
        else if (DBDataType_Integer == m_cell.type) {
            if (m_cell.data.l != x.m_cell.data.l) {
                return false;
            }
        }
        else if (DBDataType_Float == m_cell.type) {
            if (m_cell.data.d != x.m_cell.data.d) {
                return false;
            }
        }

        return true;
    }

    bool DBDataCell::operator!=(const DBDataCell& x) const
    {
        // LOGD("DBDataCell::operator!=(const DBDataCell& x)");
        if (m_cell.type != x.m_cell.type) {
            return true;
        }

        if (DBDataType_String == m_cell.type
            || DBDataType_Blob == m_cell.type) {
            if (m_cell.data.buffer.size != x.m_cell.data.buffer.size) {
                return true;
            }

            if (NULL == m_cell.data.buffer.ptr
                || NULL == x.m_cell.data.buffer.ptr) {
                return true;
            }
            int r = memcmp(m_cell.data.buffer.ptr,
                    x.m_cell.data.buffer.ptr,
                    m_cell.data.buffer.size);
            if (r) {
                return true;
            }
        }
        else if (DBDataType_Integer == m_cell.type) {
            if (m_cell.data.l != x.m_cell.data.l) {
                return true;
            }
        }
        else if (DBDataType_Float == m_cell.type) {
            if (m_cell.data.d != x.m_cell.data.d) {
                return true;
            }
        }

        return false;
    }


    DBDataCell& DBDataCell::operator=(const DBDataCell& x)
    {
        // LOGD("DBDataCell::operator=(const DBDataCell& x)");
        if (this != &x) {
            DBDATA_CELL_FREE(m_cell);

            memcpy(&m_cell, &(x.m_cell), sizeof(Cell));
            if ((DBDataType_String == m_cell.type || DBDataType_Blob == m_cell.type)
                && x.m_cell.data.buffer.ptr) {
                m_cell.data.buffer.ptr = malloc(m_cell.data.buffer.size);
                // LOGD("+++malloced %p, size[%u]", m_cell.data.buffer.ptr, m_cell.data.buffer.size);
                if (NULL != m_cell.data.buffer.ptr) {
                    memcpy(m_cell.data.buffer.ptr,
                            x.m_cell.data.buffer.ptr,
                            x.m_cell.data.buffer.size);
                }
                else {
                    // error
                    m_cell.type = DBDataType_Null;
                }
            }
        }

        return *this;
    }

    bool DBDataCell::putBlob(const void* value, size_t size)
    {
        // LOGD("DBDataCell::putBlob");
        if (NULL == value || 0 == size) {
            return false;
        }

        void* temp = malloc(size);
        // LOGD("+++malloced %p, size[%u]", temp, size);
        if (NULL == temp) {
            return false;
        }
        memcpy(temp, value, size);

        DBDATA_CELL_FREE(m_cell);

        m_cell.type = DBDataType_Blob;
        m_cell.data.buffer.ptr = temp;
        m_cell.data.buffer.size = size;

        return true;
    }

    bool DBDataCell::putString(const char* value, size_t length)
    {
        // LOGD("DBDataCell::putString");
        if (NULL == value || 0 == length) {
            return false;
        }

        int len = strlen(value);
        if (len > length) {
            len = length;
        }
        void* temp = malloc(len + 1);
        // LOGD("+++malloced %p, size[%u]", temp, len + 1);
        if (NULL == temp) {
            return false;
        }
        memset(temp, 0x00, len + 1);
        memcpy(temp, value, len);

        DBDATA_CELL_FREE(m_cell);

        m_cell.type = DBDataType_String;
        m_cell.data.buffer.ptr = temp;
        m_cell.data.buffer.size = len + 1;

        return true;
    }

    bool DBDataCell::putLong(int value)
    {
        // LOGD("DBDataCell::putLong(%lld)", value);
        DBDATA_CELL_FREE(m_cell);

        m_cell.type = DBDataType_Integer;
        m_cell.data.l = value;

        return true;
    }

    bool DBDataCell::putDouble(double value)
    {
        // LOGD("DBDataCell::putdouble(%lf)", value);
        DBDATA_CELL_FREE(m_cell);

        m_cell.type = DBDataType_Float;
        m_cell.data.d = value;

        return true;
    }

    bool DBDataCell::putNull()
    {
        // LOGD("DBDataCell::putNull");
        DBDATA_CELL_FREE(m_cell);

        m_cell.type = DBDataType_Null;

        return true;
    }

    DBDataType DBDataCell::type() const
    {
        // LOGD("cell type is %d", m_cell.type);
        return m_cell.type;
    }

    int DBDataCell::getLong() const
    {
        // LOGD("DBDataCell::getLong");
        if (m_cell.type == DBDataType_Integer) {
            // LOGD("cell data is %lld", m_cell.data.l);
            return m_cell.data.l;
        }

        return 0;
    }

    double DBDataCell::getDouble() const
    {
        // LOGD("DBDataCell::getdouble()");
        if (m_cell.type == DBDataType_Float) {
            // LOGD("cell data is %lf", m_cell.data.d);
            return m_cell.data.d;
        }

        return 0;
    }

    const char* DBDataCell::getString(size_t& length) const
    {
        // LOGD("DBDataCell::getString");
        if (m_cell.type == DBDataType_String) {
            if (0 < m_cell.data.buffer.size) {
                length = m_cell.data.buffer.size - 1;
            }
            else {
                length = m_cell.data.buffer.size;
            }
            // LOGD("cell data is [%s]", (char*)m_cell.data.buffer.ptr);
            return (char*)m_cell.data.buffer.ptr;
        }

        return NULL;
    }

    const void* DBDataCell::getBlob(size_t& outSize) const
    {
        // LOGD("DBDataCell::getBlob");
        if (m_cell.type == DBDataType_Blob) {
            outSize = m_cell.data.buffer.size;
            // LOGD("cell data is %p, size is %zu", m_cell.data.buffer.ptr, outSize);
            return m_cell.data.buffer.ptr;
        }

        return NULL;
    }

    DBDataRow::DBDataRow(int columount)
          : m_count(columount)
          , m_cells(NULL)
    {
        // LOGD("DBDataRow::DBDataRow(%u)", columount);
        if (m_count > 0) {
            m_cells = new DBDataCell[m_count];
            if (NULL == m_cells) {
                // error
                m_count = 0;
            }
        }
    }

    DBDataRow::DBDataRow(const DBDataRow& x)
          : m_count(x.m_count)
          , m_cells(NULL)
    {
        // LOGD("DBDataRow::DBDataRow(const DBDataRow& x)");
        if (m_count > 0 && NULL != x.m_cells) {
            m_cells = new DBDataCell[m_count];
            if (NULL != m_cells) {
                for (int i = 0; i < m_count; i++) {
                    m_cells[i] = x.m_cells[i];
                }
            }
            else {
                // error
                m_count = 0;
            }
        }
        else {
            m_count = 0;
        }
    }

    DBDataRow::~DBDataRow()
    {
        // LOGD("DBDataRow::~DBDataRow()");
        if (m_cells) {
            delete[] m_cells;
            m_cells = NULL;
        }
    }

    bool DBDataRow::operator==(const DBDataRow& x) const
    {
        // LOGD("DBDataRow::operator==(const DBDataRow& x)");
        if (m_count != x.m_count) {
            return false;
        }

        if (m_count > 0) {
            for (int i = 0; i < m_count; i++) {
                if (m_cells[i] != x.m_cells[i]) {
                    return false;
                }
            }
        }

        return true;
    }

    DBDataRow& DBDataRow::operator=(const DBDataRow& x)
    {
        // LOGD("DBDataRow::operator=(const DBDataRow& x)");
        if (this != &x) {
            if (m_cells) {
                delete[] m_cells;
                m_cells = NULL;
            }

            m_count = x.m_count;
            if (m_count > 0) {
                m_cells = new DBDataCell[m_count];
                for (int i = 0; i < m_count; i++) {
                    m_cells[i] = x.m_cells[i];
                }
            }
        }

        return *this;
    }

    int DBDataRow::getColumnCount() const
    {
        // LOGD("column count is %u", m_count);
        return m_count;
    }

    DBDataType DBDataRow::type(int index) const
    {
        // LOGD("DBDataRow::type(%u)", index);
        if (index >= m_count) {
            return DBDataType_Null;
        }

        if (NULL == m_cells) {
            return DBDataType_Null;
        }

        return m_cells[index].type();
    }

    bool DBDataRow::putBlob(int index, const void* value, size_t size, const char* name)
    {
        // LOGD("DBDataRow::putBlob(index=%u, value=%p, size=%zu)", index, value, size);
        if (index >= m_count || NULL == value || 0 == size) {
            return false;
        }

        if (NULL == m_cells) {
            return false;
        }

        if (name != NULL) {
            m_cells[index].setName(name);
        }

        return m_cells[index].putBlob(value, size);
    }

    bool DBDataRow::putString(int index, const char* value, size_t length, const char* name)
    {
        // LOGD("DBDataRow::putString(index=%u, value=[%s], length=%zu)", index, value, length);
        if (index >= m_count || NULL == value || 0 == length) {
            return false;
        }

        if (NULL == m_cells) {
            return false;
        }

        if (name != NULL) {
            m_cells[index].setName(name);
        }

        return m_cells[index].putString(value, length);
    }

    bool DBDataRow::putLong(int index, int value, const char* name)
    {
        // LOGD("DBDataRow::putLong(index=%u, value=%lld)", index, value);
        if (index >= m_count) {
            return false;
        }

        if (NULL == m_cells) {
            return false;
        }

        if (name != NULL) {
            m_cells[index].setName(name);
        }

        return m_cells[index].putLong(value);
    }

    bool DBDataRow::putDouble(int index, double value, const char* name)
    {
        // LOGD("DBDataRow::putdouble(index=%u, value=%lf)", index, value);
        if (index >= m_count) {
            return false;
        }

        if (NULL == m_cells) {
            return false;
        }

        if (name != NULL) {
            m_cells[index].setName(name);
        }

        return m_cells[index].putDouble(value);
    }

    bool DBDataRow::putNull(int index, const char* name)
    {
        // LOGD("DBDataRow::putNull(index=%u)", index);
        if (index >= m_count) {
            return false;
        }

        if (NULL == m_cells) {
            return false;
        }

        if (name != NULL) {
            m_cells[index].setName(name);
        }

        return m_cells[index].putNull();
    }

    int DBDataRow::getLong(int index) const
    {
        // LOGD("DBDataRow::getLong(index=%u)", index);
        if (index >= m_count) {
            return 0;
        }

        if (NULL == m_cells) {
            return 0;
        }

        return m_cells[index].getLong();
    }

    double DBDataRow::getDouble(int index) const
    {
        // LOGD("DBDataRow::getdouble(index=%u)", index);
        if (index >= m_count) {
            return 0;
        }

        if (NULL == m_cells) {
            return 0;
        }

        return m_cells[index].getDouble();
    }

    const char* DBDataRow::getString(int index, size_t& length) const
    {
        // LOGD("DBDataRow::getString(index=%u, length=%zu)", index, length);
        if (index >= m_count) {
            return NULL;
        }

        if (NULL == m_cells) {
            return NULL;
        }

        return m_cells[index].getString(length);
    }

    const void* DBDataRow::getBlob(int index, size_t& size) const
    {
        // LOGD("DBDataRow::getBlob(index=%u, size=%zu)", index, size);
        if (index >= m_count) {
            return NULL;
        }

        if (NULL == m_cells) {
            return NULL;
        }

        return m_cells[index].getBlob(size);
    }

    const std::string& DBDataRow::getColumnName(int index) const
    {
        if (index < m_count) {
            return m_cells[index].getName();
        }

        return "";
    }

    DBDataTable::DBDataTable(int columount)
          : m_rowSpList()
          , m_columnCount(columount)
          , m_columnTypes(NULL)
    {
        // LOGD("DBDataTable::DBDataTable(%u)", columount);
        if (columount > 0) {
            m_columnTypes = (DBDataType*)malloc(sizeof(DBDataType) * columount);
            // LOGD("+++malloced %p, size[%u]", m_columnTypes, sizeof(DBDataType) * columount);
            for (int i = 0; i < columount; i++) {
                m_columnTypes[i] = DBDataType_Null;
            }
        }
    }

    DBDataTable::DBDataTable(int rowCount, int columount)
          : m_rowSpList()
          , m_columnCount(columount)
          , m_columnTypes(NULL)
    {
        // LOGD("DBDataTable::DBDataTable(%u, %u)", rowCount, columount);
        if (columount > 0) {
            m_columnTypes = (DBDataType*)malloc(sizeof(DBDataType) * columount);
            // LOGD("+++malloced %p, size[%u]", m_columnTypes, sizeof(DBDataType) * columount);
            for (int i = 0; i < columount; i++) {
                m_columnTypes[i] = DBDataType_Null;
            }
        }

        m_rowSpList.resize(rowCount, NULL);
    }

    DBDataTable::~DBDataTable()
    {
        // LOGD("DBDataTable::~DBDataTable()");
        reset();
    }

    bool DBDataTable::setRowCount(int rowCount)
    {
        // LOGD("DBDataTable::setRowCount(%u)", rowCount);
        m_rowSpList.resize(rowCount, NULL);
        return true;
    }

    int DBDataTable::getRowCount()
    {
        // LOGD("row count is %u", m_rowSpList.size());
        return m_rowSpList.size();
    }

    bool DBDataTable::setColumnCount(int columount)
    {
        // LOGD("DBDataTable::setColumount(%u)", columount);
        if (m_columnCount > 0) {
            return false;
        }

        m_columnCount = columount;
        if (columount > 0) {
            m_columnTypes = (DBDataType*)malloc(sizeof(DBDataType) * columount);
            // LOGD("+++malloced %p, size[%u]", m_columnTypes, sizeof(DBDataType) * columount);
            for (int i = 0; i < columount; i++) {
                m_columnTypes[i] = DBDataType_Null;
            }
        }
        return true;
    }

    int DBDataTable::getColumnCount()
    {
        // LOGD("column count is %u", m_columnCount);
        return m_columnCount;
    }

    bool DBDataTable::setColumnType(int column, DBDataType type)
    {
        // LOGD("DBDataTable::setColumnType(column=%u, type=%d)", column, type);
        if (column >= m_columnCount) {
            return false;
        }

        if (m_columnTypes[column] != DBDataType_Null) {
            return false;
        }

        m_columnTypes[column] = type;

        return true;
    }

    DBDataType DBDataTable::getColumnType(int column)
    {
        // LOGD("DBDataTable::getColumnType(column=%u)", column);
        if (NULL != m_columnTypes && column < m_columnCount) {
            // LOGD("column type is %d", m_columnTypes[column]);
            return m_columnTypes[column];
        }
        else {
            return DBDataType_Null;
        }
    }

    bool DBDataTable::reset()
    {
        // LOGD("DBDataTable::reset");
        m_rowSpList.resize(0);

        if (m_columnTypes) {
            free(m_columnTypes);
            // LOGD("---free %p, size[%u]", m_columnTypes, sizeof(DBDataType) * m_columnCount);
            m_columnTypes = NULL;
        }

        return true;
    }

    void DBDataTable::addRow()
    {
        // LOGD("DBDataTable::addRow");
        m_rowSpList.resize(m_rowSpList.size() + 1);
    }

    bool DBDataTable::putBlob(int row, int column, const void* value, size_t size)
    {
        // LOGD("DBDataTable::putBlob(row=%u, column=%u, value=%p, size=%zu)", row, column, value, size);
        if (row >= m_rowSpList.size()
            || column >= m_columnCount
            || NULL == value
            || 0 == size) {
            return false;
        }

        if (getColumnType(column) != DBDataType_Null
            && getColumnType(column) != DBDataType_Blob) {
            return false;
        }

        if (NULL == m_rowSpList[row]) {
            m_rowSpList[row] = new DBDataRow(m_columnCount);
            if (NULL == m_rowSpList[row]) {
                return false;
            }
        }

        return m_rowSpList[row]->putBlob(column, value, size);
    }

    bool DBDataTable::putString(int row, int column, const char* value, size_t size)
    {
        // LOGD("DBDataTable::putString(row=%u, column=%u, value=[%s], size=%zu)", row, column, value, size);
        if (row >= m_rowSpList.size()
            || column >= m_columnCount
            || NULL == value
            || 0 == size) {
            return false;
        }

        if (getColumnType(column) != DBDataType_Null
            && getColumnType(column) != DBDataType_String) {
            return false;
        }

        if (NULL == m_rowSpList[row]) {
            m_rowSpList[row] = new DBDataRow(m_columnCount);
            if (NULL == m_rowSpList[row]) {
                return false;
            }
        }

        return m_rowSpList[row]->putString(column, value, size);
    }

    bool DBDataTable::putLong(int row, int column, int value)
    {
        // LOGD("DBDataTable::putLong(row=%u, column=%u, value=%lld)", row, column, value);
        if (row >= m_rowSpList.size() || column >= m_columnCount) {
            return false;
        }

        if (getColumnType(column) != DBDataType_Null
            && getColumnType(column) != DBDataType_Integer) {
            return false;
        }

        if (NULL == m_rowSpList[row]) {
            m_rowSpList[row] = new DBDataRow(m_columnCount);
            if (NULL == m_rowSpList[row]) {
                return false;
            }
        }

        return m_rowSpList[row]->putLong(column, value);
    }

    bool DBDataTable::putDouble(int row, int column, double value)
    {
        // LOGD("DBDataTable::putdouble(row=%u, column=%u, value=%lf)", row, column, value);
        if (row >= m_rowSpList.size() || column >= m_columnCount) {
            return false;
        }

        if (getColumnType(column) != DBDataType_Null
            && getColumnType(column) != DBDataType_Float) {
            return false;
        }

        if (NULL == m_rowSpList[row]) {
            m_rowSpList[row] = new DBDataRow(m_columnCount);
            if (NULL == m_rowSpList[row]) {
                return false;
            }
        }

        return m_rowSpList[row]->putDouble(column, value);
    }

    bool DBDataTable::putNull(int row, int column)
    {
        // LOGD("DBDataTable::putNull(row=%u, column=%u)", row, column);
        if (row >= m_rowSpList.size() || column >= m_columnCount) {
            return false;
        }

        if (NULL == m_rowSpList[row]) {
            m_rowSpList[row] = new DBDataRow(m_columnCount);
            if (NULL == m_rowSpList[row]) {
                return false;
            }
        }

        return m_rowSpList[row]->putNull(column);
    }

    DBDataType DBDataTable::getType(int row, int column)
    {
        // LOGD("DBDataTable::getType(row=%u, column=%u)", row, column);
        if (row >= m_rowSpList.size() || column >= m_columnCount) {
            return DBDataType_Null;
        }

        if (NULL == m_rowSpList[row]) {
            return DBDataType_Null;
        }

        return m_rowSpList[row]->type(column);
    }

    int DBDataTable::getLong(int row, int column)
    {
        // LOGD("DBDataTable::getLong(row=%u, column=%u)", row, column);
        if (row >= m_rowSpList.size() || column >= m_columnCount) {
            return 0;
        }

        if (NULL == m_rowSpList[row]) {
            return 0;
        }

        if (m_rowSpList[row]->type(column) == DBDataType_Integer) {
            return m_rowSpList[row]->getLong(column);
        }

        return 0;
    }

    double DBDataTable::getDouble(int row, int column)
    {
        // LOGD("DBDataTable::getdouble(row=%u, column=%u)", row, column);
        if (row >= m_rowSpList.size() || column >= m_columnCount) {
            return 0;
        }

        if (NULL == m_rowSpList[row]) {
            return 0;
        }

        if (m_rowSpList[row]->type(column) == DBDataType_Float) {
            return m_rowSpList[row]->getDouble(column);
        }

        return 0;
    }

    const char* DBDataTable::getString(int row, int column, size_t& length)
    {
        // LOGD("DBDataTable::getString(row=%u, column=%u)", row, column);
        if (row >= m_rowSpList.size() || column >= m_columnCount) {
            return NULL;
        }

        if (NULL == m_rowSpList[row]) {
            return NULL;
        }

        if (m_rowSpList[row]->type(column) == DBDataType_String) {
            return m_rowSpList[row]->getString(column, length);
        }

        return NULL;
    }

    const void* DBDataTable::getBlob(int row, int column, size_t& size)
    {
        // LOGD("DBDataTable::getBlob(row=%u, column=%u)", row, column);
        if (row >= m_rowSpList.size() || column >= m_columnCount) {
            return NULL;
        }

        if (NULL == m_rowSpList[row]) {
            return NULL;
        }

        if (m_rowSpList[row]->type(column) == DBDataType_Blob) {
            return m_rowSpList[row]->getBlob(column, size);
        }

        return NULL;
    }

} /* namespace nutshell */
/* EOF */
