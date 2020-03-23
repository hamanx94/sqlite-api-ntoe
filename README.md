# sqlite-api-ntoe
> Job accumulation  

* create
```cpp
bool sql_table_create(
    char *dbfilename, 
    char *tablename, 
    int FieldCount, 
    tFIELD_DEF *stFieldDefs, 
    char *err);
```

* read
```cpp
bool sql_field_read(
    char *dbfilename, 
    char *tablename, 
    FieldName *fieldNameList, 
    int fieldNameCount, 
    FieldData *fieldDataList, 
    int *fieldDataCount, 
    char *condition, 
    char *err);
```

* write
```cpp
bool sql_data_write(
    char *dbfilename, 
    char *tablename, 
    tFIELD *fieldList, 
    int fieldCount, 
    char *err);
```

* update
```cpp
bool sql_data_update(
    char *dbfilename, 
    char *tablename, 
    tFIELD *fieldList, 
    int fieldCount, 
    char *condition, 
    char *err);
```
