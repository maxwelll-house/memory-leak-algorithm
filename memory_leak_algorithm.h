#ifndef __ML_AlGORITHM__
#include <assert.h>

#define MAX_STRUCTURE_NAME_SIZE 128
#define MAX_FIELD_NAME_SIZE 128

typedef enum {
  UINT8,
  UINT32,
  INT32,
  CHAR,
  OBJ_PTR,
  FLOAT,
  DOUBLE,
  OBJ_STRUCT,
} data_type_t;

typedef enum {
  ALG_FALSE,
  ALG_TRUE
} boolean_t;

#define OFFSETOF(struct_name, fld_name)     \
    (unsigned long)&(((struct_name *)0)->fld_name)

#define FIELD_SIZE(struct_name, fld_name)   \
    sizeof(((struct_name *)0)->fld_name)

typedef struct _struct_db_rec_ struct_db_rec_t;

// Information of any field of the structure
typedef struct _field_info_{
    char fname [MAX_FIELD_NAME_SIZE];   // Name of the field
    data_type_t dtype;                  // Data type of the field
    unsigned int size;                  // Size of the field*/
    unsigned int offset;                // Offset of the field*/
    char nested_str_name[MAX_STRUCTURE_NAME_SIZE];
} field_info_t;

// Information about structure
struct _struct_db_rec_{
    struct_db_rec_t *next;  // Pointer to the next structure in the linked list
    char struct_name [MAX_STRUCTURE_NAME_SIZE];
    unsigned int ds_size;   // Size of the structure
    unsigned int n_fields;  //Number of fields in the structure
    field_info_t *fields;   // pointer to the array of fields
};

// Database of structures with pointer on head of it
typedef struct _struct_db_{
    struct_db_rec_t *head;
    unsigned int count;
} struct_db_t;

//Functions to print structure records and structure database
void print_structure_rec (struct_db_rec_t *struct_rec);
void print_structure_db (struct_db_t *struct_db);

// Function to add a record in a database
int add_structure_to_struct_db(struct_db_t *struct_db, struct_db_rec_t *struct_rec);

//Structure registration macro
#define FIELD_INFO(struct_name, fld_name, dtype, nested_struct_name)    \
  { #fld_name, dtype, FIELD_SIZE(struct_name, fld_name),                \
      OFFSETOF(struct_name, fld_name), #nested_struct_name}

#define REG_STRUCT(struct_db, st_name, fields_arr)                    \
    do {                                                              \
        struct_db_rec_t *rec = calloc(1, sizeof(struct_db_rec_t));    \
        strncpy(rec->struct_name, #st_name, MAX_STRUCTURE_NAME_SIZE); \
        rec->ds_size = sizeof(st_name);                               \
        rec->n_fields = sizeof(fields_arr)/sizeof(field_info_t);      \
        rec->fields = fields_arr;                                     \
        if(add_structure_to_struct_db(struct_db, rec)){               \
            assert(0);                                                \
        }                                                             \
    } while(0);


typedef struct _object_db_rec_ object_db_rec_t;

//Structure to present graph node
struct _object_db_rec_{
  object_db_rec_t *next;
  void *ptr;
  unsigned int units;
  struct_db_rec_t *struct_rec;
  boolean_t is_visited;
  boolean_t is_root;
};

typedef struct _object_db_{
  struct_db_t *struct_db;
  object_db_rec_t *head;
  unsigned int count;
} object_db_t;

// Printing graph node information
void print_object_rec (object_db_rec_t *object_rec, int i);
void print_object_db (object_db_t *object_db);

//Custom function to malloc the memory for the object*/
void* custom_calloc (object_db_t *object_db, char* struct_name, int units);

//Functions to rule root objects in structures graph
void register_root_object (object_db_t *object_db, void *objptr, char* struct_name, unsigned int units);
void set_object_as_global_root (object_db_t *object_db, void *obj_ptr);

//Functions to run the algorithm
void run_algorithm(object_db_t *object_db);

void init_primitive_data_types_support (struct_db_t* struct_db);

void set_dynamic_object_as_root(object_db_t *object_db, void *obj_ptr);

void report_leaked_objects(object_db_t *object_db);

#endif /* __ML_AlGORITHM__ */
