#include "memory_leak_algorithm.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Example structures for graph and algorithm demonstration
typedef struct human_ {
    char human_name[30];
    unsigned int human_id;
    unsigned int age;
    struct human_ *mgr;
    float salary;
    int *p;
} human_t;

typedef struct unit_{
    char unit_name[32];
    unsigned int rollno;
    unsigned int age;
    float aggregate;
    struct unit_ *best_friend;
} unit_t;

int main(int argc, char **argv) {
  // Main func used to illustrate the algorithm
  // New structure database
  struct_db_t *struct_db = calloc(1, sizeof(struct_db_t));
  init_primitive_data_types_support (struct_db);

  // Create a record for structure
  static field_info_t human_fields[] = {
        FIELD_INFO(human_t, human_name, CHAR,    0),
        FIELD_INFO(human_t, human_id,   UINT32,  0),
        FIELD_INFO(human_t, age,      UINT32,  0),
        FIELD_INFO(human_t, mgr,      OBJ_PTR, human_t),
        FIELD_INFO(human_t, salary,   FLOAT, 0),
        FIELD_INFO(human_t, p,      OBJ_PTR, 0)
    };
  REG_STRUCT(struct_db, human_t, human_fields);

 // Create another record for structure
  static field_info_t unit_fields[] = {
        FIELD_INFO(unit_t, unit_name, CHAR, 0),
        FIELD_INFO(unit_t, rollno,    UINT32, 0),
        FIELD_INFO(unit_t, age,       UINT32, 0),
        FIELD_INFO(unit_t, aggregate, FLOAT, 0),
        FIELD_INFO(unit_t, best_friend, OBJ_PTR, unit_t)
    };
  REG_STRUCT(struct_db, unit_t, unit_fields);

  // There you can see a database
  print_structure_db(struct_db);

  // Initialize a new Object database to present structure database
  object_db_t *object_db = calloc(1, sizeof(object_db_t));
  object_db->struct_db = struct_db;

  // Some sample objectcs to show the result of detection
  unit_t *first_temp = custom_calloc(object_db, "unit_t", 1);
  set_dynamic_object_as_root (object_db, first_temp);

  unit_t *second_temp = custom_calloc(object_db, "unit_t", 1);
  strncpy(second_temp->unit_name, "second_temp", strlen("second_temp"));
  //EXAMPLE 1 : To see the leak - you can comment the line bellow
  first_temp->best_friend = second_temp;

  human_t *first_human = custom_calloc(object_db, "human_t", 1);
  //EXAMPLE 2 : To see the leak - you can comment the line bellow
  set_dynamic_object_as_root (object_db, first_human);
  first_human->p=custom_calloc(object_db, "int", 1);
  //EXAMPLE 3 : To see the leak - you can uncomment the line bellow
  //first_human->p=NULL;

  print_object_db(object_db);

  run_algorithm(object_db);
  printf("Leaked objects:  \n");
  report_leaked_objects(object_db);
  system("pause");
  return 0;
}
