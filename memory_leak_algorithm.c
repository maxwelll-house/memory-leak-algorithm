#include <stdio.h>
#include <stdlib.h>
#include "memory_leak_algorithm.h"
#include <assert.h>
#include <memory.h>
#include <string.h>

char *DATA_TYPE[] = {"UINT8", "UINT32", "INT32",
                     "CHAR", "OBJ_PTR", "VOID_PTR", "FLOAT",
                     "DOUBLE", "OBJ_STRUCT"};


void print_structure_rec(struct_db_rec_t *struct_rec) {
  if (!struct_rec) {
    return;
  }
  int j = 0;
  field_info_t *field = NULL;
  printf("|------------------------------------------------------|\n");
  printf("| %-20s | size = %-8d | #flds = %-3d |\n", struct_rec->struct_name, struct_rec->ds_size, struct_rec->n_fields);
  printf("|------------------------------------------------------|------------------------------------------------------------------------------------------|\n");
  for(j = 0; j < struct_rec->n_fields; j++) {
      field = &struct_rec->fields[j];
      printf("  %-20s |", "");
      printf("%-3d %-20s | dtype = %-15s | size = %-5d | offset = %-6d|  nstructname = %-20s  |\n",
              j, field->fname, DATA_TYPE[field->dtype], field->size, field->offset, field->nested_str_name);
      printf("  %-20s |", "");
      printf("--------------------------------------------------------------------------------------------------------------------------|\n");
  }
}

void print_structure_db(struct_db_t *struct_db) {
  if(!struct_db) {
    return;
  }
 printf("printing STRUCURE DATABASE\n");
 int i = 0;
 struct_db_rec_t *struct_rec = NULL;
 struct_rec = struct_db->head;
 printf("No of Structures Registered = %d\n", struct_db->count);
 while(struct_rec) {
     printf("structure No : %d (%p)\n", i++, struct_rec);
     print_structure_rec(struct_rec);
     struct_rec = struct_rec->next;
   }
}

int add_structure_to_struct_db(struct_db_t *struct_db, struct_db_rec_t *struct_rec) {
    struct_db_rec_t *head = struct_db->head;
    if(!head) {
        struct_db->head = struct_rec;
        struct_rec->next = NULL;
        struct_db->count++;
        return 0;
    }
    struct_rec->next = head;
    struct_db->head = struct_rec;
    struct_db->count++;
    return 0;
}

static struct_db_rec_t *struct_db_look_up(struct_db_t *struct_db, char *struct_name){
    struct_db_rec_t *head = struct_db->head;
    if(!head) {
       return NULL;
    }
    for(; head; head = head->next){
        if(strncmp(head->struct_name, struct_name, MAX_STRUCTURE_NAME_SIZE) ==0)
            return head;
    }
    return NULL;
}


static object_db_rec_t *object_db_look_up(object_db_t *object_db, void *ptr){

    object_db_rec_t *head = object_db->head;
    if(!head) {
      return NULL;
    }

    for(; head; head = head->next){
        if(head->ptr == ptr)
            return head;
    }
    return NULL;
}

static void add_object_to_object_db (object_db_t *object_db,
                     void *ptr,
                     int units,
                     struct_db_rec_t *struct_rec,
                     boolean_t is_root) {
  object_db_rec_t *obj_rec = object_db_look_up(object_db, ptr);
  //Do not add same object twice
  assert(!obj_rec);

  obj_rec = calloc(1, sizeof(object_db_rec_t));
  obj_rec->next = NULL;
  obj_rec->ptr = ptr;
  obj_rec->units = units;
  obj_rec->struct_rec = struct_rec;
  obj_rec->is_root = is_root;

  object_db_rec_t *head = object_db->head;

  if (!head) {
    object_db->head = obj_rec;
    obj_rec->next = NULL;
    object_db->count++;
    return;
  }

  obj_rec->next = head;
  object_db->head = obj_rec;
  object_db->count++;
}

void* custom_calloc (object_db_t *object_db, char* struct_name, int units) {
  struct_db_rec_t *struct_rec = struct_db_look_up(object_db->struct_db, struct_name);
  assert(struct_rec);
  void *ptr = calloc(units, struct_rec->ds_size);
  add_object_to_object_db(object_db, ptr, units, struct_rec, ALG_FALSE);
  return ptr;
}

void print_object_rec(object_db_rec_t *obj_rec, int i) {
    if(!obj_rec) {
      return;
    }
    printf( "-----------------------------------------------------------------------------------------------------|\n");
    printf( "%-3d ptr = %-10p | next = %-10p | units = %-4d | struct_name = %-10s | is_root = %s |\n",
        i, obj_rec->ptr, obj_rec->next, obj_rec->units, obj_rec->struct_rec->struct_name, obj_rec->is_root ? "TRUE " : "FALSE");
    printf( "-----------------------------------------------------------------------------------------------------|\n");
}

void print_object_db(object_db_t *object_db) {
    object_db_rec_t *head = object_db->head;
    unsigned int i = 0;
    printf("Printing OBJECT DATABASE\n");
    for(; head; head = head->next){
        print_object_rec(head, i++);
    }
}

//The global object of the application which is not created by custom calloc function
void register_global_object_as_root (object_db_t *object_db, void *objptr, char *struct_name, unsigned int units) {
    struct_db_rec_t *struct_rec = struct_db_look_up(object_db->struct_db, struct_name);
    assert(struct_rec);
    add_object_to_object_db(object_db, objptr, units, struct_rec, ALG_TRUE);
}

void set_object_as_global_root(object_db_t *object_db, void *obj_ptr) {
    object_db_rec_t *obj_rec = object_db_look_up(object_db, obj_ptr);
    assert(obj_rec);

    obj_rec->is_root = ALG_TRUE;
}

void set_dynamic_object_as_root(object_db_t *object_db, void *obj_ptr) {
    object_db_rec_t *obj_rec = object_db_look_up(object_db, obj_ptr);
    assert(obj_rec);

    obj_rec->is_root = ALG_TRUE;
}


static object_db_rec_t *get_next_root_object(object_db_t *object_db, object_db_rec_t *starting_from_here) {
    object_db_rec_t *first = starting_from_here ? starting_from_here->next : object_db->head;
    while(first) {
        if(first->is_root)
        {
            return first;
        }
        first = first->next;
    }
    return NULL;
}

static void init_algorithm(object_db_t *object_db){
     object_db_rec_t *obj_rec = object_db->head;
     while(obj_rec) {
         obj_rec->is_visited = ALG_FALSE;
         obj_rec = obj_rec->next;
     }
}

static void explore_objects_recursively(object_db_t *object_db, object_db_rec_t *parent_obj_rec){
    unsigned int i , n_fields;
    char *parent_obj_ptr = NULL,
         *child_obj_offset = NULL;
    void *child_object_address = NULL;
    field_info_t *field_info = NULL;

    object_db_rec_t *child_object_rec = NULL;
    struct_db_rec_t *parent_struct_rec = parent_obj_rec->struct_rec;

    //Parent object must have already visited
    assert(parent_obj_rec->is_visited);

    if(parent_struct_rec->n_fields == 0){
        return;
    }

    for( i = 0; i < parent_obj_rec->units; i++){

        parent_obj_ptr = (char *)(parent_obj_rec->ptr) + (i * parent_struct_rec->ds_size);

        for(n_fields = 0; n_fields < parent_struct_rec->n_fields; n_fields++) {
            field_info = &parent_struct_rec->fields[n_fields];
            switch(field_info->dtype){
                case UINT8:
                case UINT32:
                case INT32:
                case CHAR:
                case FLOAT:
                case DOUBLE:
                case OBJ_STRUCT:
                    break;
                case OBJ_PTR:
                default:
                  ;

                // child_obj_offset is the memory location inside parent object where address of next level object is stored*/
                child_obj_offset = parent_obj_ptr + field_info->offset;
                memcpy(&child_object_address, child_obj_offset, sizeof(void *));

                // child_object_address now stores the address of the next object in the graph
                if (!child_object_address) {
                  continue;
                }

                child_object_rec = object_db_look_up(object_db, child_object_address);
                assert(child_object_rec);

                // If it is able to reach this child object "child_object_rec"
                // from parent object mark this child object as visited and explore its children recirsively.
                // If this child object is already visited, then do nothing - avoid loops
                if(!child_object_rec->is_visited) {
                    child_object_rec->is_visited = ALG_TRUE;
                }
                else{
                    continue; //Do nothing, explore next child object
                }
            }
        }
    }
}

void run_algorithm (object_db_t *object_db) {
  // All objects in object databse as unvisited in the beginning
  init_algorithm(object_db);
  // Getting the first root object from the object db - it could be elsewhere in graph
  // There can be multiple root objects in algorithm
  object_db_rec_t *root_obj = get_next_root_object(object_db, NULL);

  while(root_obj) {
    if (root_obj->is_visited) {
      // All objects reachable from this root_obj has already been explored, go further
      root_obj - get_next_root_object(object_db, root_obj);
      continue;
    }

    root_obj->is_visited = ALG_TRUE;
    explore_objects_recursively(object_db, root_obj);
    root_obj = get_next_root_object(object_db, root_obj);
  }
}

//Functions for primitive data types
void init_primitive_data_types_support (struct_db_t* struct_db) {
  REG_STRUCT(struct_db, int, 0);
  REG_STRUCT(struct_db, float, 0);
  REG_STRUCT(struct_db, double, 0);
}

void report_leaked_objects(object_db_t *object_db) {
    int i = 0;
    object_db_rec_t *head;

    printf("Dumping Leaked Objects\n");

    for(head = object_db->head; head; head = head->next){
        if(!head->is_visited) {
            print_object_rec(head, i++);
            printf("\n\n");
        }
    }
}
