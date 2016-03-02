#include <iostream>
#include "database.h"

using namespace sql;

void printResult(DBDataTable *t)
{
    if (t == NULL) {
        std::cout << "search result is NULL.\n";
        return ;
    }

    int count = t->getRowCount();
    std::cout << "row count is " << count << "\n";

    for (int j = 0; j < count; j++) {
        int columnCount = t->getColumnCount();
        for (int i = 0; i < columnCount; i++) {
            DBDataType type = t->getType(j, i);
            switch (type) {
            case DBDataType_Integer:
            {
                int value = t->getLong(j, i);
                std::cout << "column " << i << " value is " << value << "\n";
                break;
            }
            case DBDataType_Float:
            {
                double value = t->getDouble(j, i);
                std::cout << "column " << i << " value is " << value << "\n";
                break;
            }
            case DBDataType_String:
            {
                size_t len = 0;
                const char* data = t->getString(j, i, len);
                std::string value(data, len);
                std::cout << "column " << i << " value is " << value << "\n";
                break;
            }
            }
        }
    }
}

int main()
{
    database db("test.db", "test123", 7);
    if (!db.isOpen()) {
        std::cout << "create or open database failed.\n";
        return 0;
    }

    int err = db.exec("CREATE TABLE TEST("\
            "ID INT PRIMARY KEY,"\
            "NAME   TEXT,"\
            "AGE    INT,"\
            "ADDRESS   CHAR(50),"\
            "SALARY    REAL);");

    if (err != 0) {
        std::cout << "database create table failed:"<< err << "\n";

        // db.close();
        // return 0;
    }

    // set default value?
    DBDataRow row(5);
    row.putLong(0, 0, "ID");
    row.putString(1, "Joy", 4, "NAME");
    row.putLong(2, 30, "AGE");
    row.putString(3, "Huaihai RD.", 12, "ADDRESS");
    row.putDouble(4, 3000.00, "SALARY");
    int result = db.insert("TEST", row);
    std::cout << "insert return value is " << result << "\n";

    DBDataRow row1(5);
    row1.putLong(0, 1, "ID");
    row1.putString(1, "Tom", 4, "NAME");
    row1.putLong(2, 34, "AGE");
    row1.putString(3, "Huaihai RD.", 12, "ADDRESS");
    row1.putDouble(4, 3500.00, "SALARY");
    result = db.insert("TEST", row1);
    std::cout << "insert return value is " << result << "\n";


    std::vector<std::string> v;
    DBDataTable *t = db.rawQuery("select * from TEST", v);
    printResult(t);

    DBDataRow row2(1);
    row2.putString(0, "Joy2", 5, "NAME");

    std::vector<std::string> args;
    args.push_back("30");

    result = db.update("TEST", row2, "AGE=?", args);
    std::cout << "update return value is " << result << "\n";

    std::vector<std::string> columns;
    columns.push_back("ID");
    columns.push_back("NAME");
    columns.push_back("AGE");
    args.clear();
    args.push_back("Huaihai RD.");
    t = db.query("TEST", columns, "ADDRESS=?", args, "NAME");
    //t = db.rawQuery("SELECT ID, NAME, AGE  FROM TEST WHERE ADDRESS=? ORDER BY AGE", args);
    printResult(t);

    result = db.remove("TEST", "ADDRESS=?", args);
    std::cout << "remove return value is " << result << "\n";

    t = db.rawQuery("select * from TEST", v);
    printResult(t);

    db.close();
}
