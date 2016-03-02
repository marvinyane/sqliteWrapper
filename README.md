sqlite wrapper

因为需要加密， 所以使用的是sqlcipher， 下载完代码后需要执行

git submodule init
git submodule update

才可以将sqlcipher下载下来。

根据sqlcipher的readme自行编译安装。

**接口使用说明**

`database(const std::string& path, const void* pKey = NULL, int nKey = 0);`

  构造函数， 打开或创建数据库， 其中pkey 和 nKey是秘钥。

`int exec(const std::string& sql);`

  执行 sql 语句， 返回值为成功或失败。 建议只用此API 创建或删除表。
  
`DBDataTable* rawQuery(const std::string& sql, const std::vector<std::string>& args);`

  查询接口， 返回值为table， 具体table的操作请参考代码。
  如果查询语句只需要绑定查询条件参数， 则使用此接口， 请确保args的个数和sql语句中where后的"?"数目相同

`DBDataTable* query(bool distinct, const std::string& table, const std::vector<std::string>& columns,
                       const std::string& where, const std::vector<std::string>& whereArgs,
                       const std::string& groupBy,
                       const std::string& having, const std::string& orderBy, const std::string& limit);`
                       
  高级查询接口， 如果查询的属性比较多， 请使用此接口， 参数的命名很直接的显示出意义。


`int insert(const std::string& table, const DBDataRow& values);`

  数据插入接口， 使用data row 来直接表示一个record。
  返回值表示插入的行号。

`int update(const std::string& table, const DBDataRow& values,
          const std::string& where, const std::vector<std::string>& whereArgs);`
          
  数据更新接口， 使用data row 表示需要更新的数据， where 和 whereArgs 来表示更新的条件。
  返回值表示更新的数据数量

`int remove(const std::string& table, const std::string& where, const std::vector<std::string>& whereArgs);`

  数据删除接口， where 和 whereArgs 表示删除的条件。
  返回值表示数据的更新数目



**TODO：**

几个地方需要换成智能指针

DBDataTable
DBDataRow
DBDataCell

需要对返回值进行整理。
