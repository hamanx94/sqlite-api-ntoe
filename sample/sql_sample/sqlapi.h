#ifndef SQLAPI_H
#define SQLAPI_H

enum DataType
{
    _INTEGER,
    _TEXT,
    _REAL,
    _BLOB,
    _NULL,
    _NUMERIC,
};

enum Constraint
{
    _NONE           = 0x00,
    _NOT_NULL       = 0x01,
    _PRIMARY_KEY    = 0x02,
    _UNIQUE         = 0x04,
    _AUTOINCREMENT  = 0x08,
};

typedef struct
{
    char FieldName[64];
    enum DataType emDataType;
    enum Constraint emConstraint;
} tFIELD_DEF;

typedef struct
{
    char FieldName[64];
    enum DataType emDataType;
    char *FieldData;
} tFIELD;

typedef char FieldName[64];
typedef char *FieldData;

#define d_FIELD_SIZE        (16)
#define d_STATUS_EMPTY      (0)
#define d_STATUS_BEEN_SET   (1)
#define d_STATUS_FULL       (2)

bool sql_db_connect(char *dbfilename, char *err);

bool sql_table_create(char *dbfilename, char *tablename, int FieldCount, tFIELD_DEF *stFieldDefs, char *err);

bool sql_field_read(char *dbfilename, char *tablename, FieldName *fieldNameList, int fieldNameCount, FieldData *fieldDataList , int *fieldDataCount, char *condition, char *err);

bool sql_data_write(char *dbfilename, char *tablename, tFIELD_DEF *stFieldDefs, int FieldNameCount, FieldData *fieldDataList, char *err);
bool sql_data_update(char *dbfilename, char *tablename, tFIELD *fieldList, int fieldCount, char *condition, char *err);

bool sql_commit(char *dbfilename, char *err);
bool sql_begin(char *dbfilename, char *err);

void sql_db_close(char *filename);

void sql_close_all();

#endif // SQLAPI_H
