# sqlite-api-ntoe
> Job accumulation  

### todo
* create
```cpp
bool sql_table_create(
    char *dbfilename, 
    char *tablename, 
    int FieldCount, 
    tFIELD_DEF *stFieldDefs, 
    char *err);
```
* write
* update
* read
```cpp
bool sql_field_read(
    char *dbfilename, 
    char *tablename, 
    char *condition, 
    int FieldCount, 
    tFIELD_IO_SET *stOutList,
    char *err);
```
