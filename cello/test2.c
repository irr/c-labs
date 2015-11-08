/*
gcc -std=gnu99 test2.c -lpthread -lm -ldl -lCello -o test
gcc -static -std=gnu99 test2.c -lCello -lpthread -lc -lm -ldl -o test
*/

#include "Cello.h"

int main(int argc, char** argv) {

  /* Shorthand $ can be used for basic types */
  var prices = new(Table, String, Int);
  set(prices, $S("Apple"),  $I(12)); 
  set(prices, $S("Banana"), $I( 6)); 
  set(prices, $S("Pear"),   $I(55));

  /* Tables also support iteration */
  foreach (key in prices) {
    var val = get(prices, key);
    print("Price of %$ is %$\n", key, val);
  }

  return 0;
}

