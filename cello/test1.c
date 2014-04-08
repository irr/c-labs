/*
gcc -std=gnu99 test1.c -lpthread -lm -ldl -lCello -o test
gcc -static -std=gnu99 test1.c -lCello -lpthread -lc -lm -ldl -o test
*/

#include "Cello.h"

int main(int argc, char** argv) {

  /* Stack objects are created using "$" */
  var int_item = $(Int, 5);
  var float_item = $(Real, 2.4);
  var string_item = $(String, "Hello");

  /* Heap objects are created using "new" */
  var items = new(List, int_item, float_item, string_item);

  /* Collections can be looped over */
  foreach (item in items) {
    /* Types are also objects */
    var type = type_of(item);
    print("Object %$ has type %$\n", item, type);
  }

  /* Heap objects destroyed with "delete" */
  delete(items); 
}