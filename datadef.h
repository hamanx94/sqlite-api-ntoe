
typedef char DataBaseName[64];
typedef char TableName[64];
typedef char FieldData[128];
//typedef char FieldDataEx;
typedef char FieldName[64];

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
    _NONE = 0x00,
    _NOT_NULL = 0x01,
    _PRIMARY_KEY = 0x02,
    _UNIQUE = 0x04,
    _AUTOINCREMENT = 0x08,
};

typedef struct
{
    char FieldName[64];
    enum DataType emDataType;
    enum Constraint emConstraint;
} FieldDef;
