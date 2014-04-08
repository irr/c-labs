/*
gcc -std=gnu99 test2.c -lpthread -lm -ldl -lCello -o test
gcc -static -std=gnu99 test2.c -lCello -lpthread -lc -lm -ldl -o test
*/

#include "Cello.h"

int main(int argc, char** argv) {

  /* Tables require "Eq" and "Hash" on key type */
  var prices = new(Table, String, Int);
  put(prices, $(String, "Apple"),  $(Int, 12)); 
  put(prices, $(String, "Banana"), $(Int,  6)); 
  put(prices, $(String, "Pear"),   $(Int, 55));

  /* Tables also supports iteration */
  foreach (key in prices) {
    var price = get(prices, key);
    print("Price of %$ is %$\n", key, price);
  }

  /* "with" automatically closes file at end of scope. */
  with (file in stream_open($(File, NULL), "prices.bin", "wb")) {

    /* First class function object */
    lambda(write_pair, args) {

      /* Run time type-checking with "cast" */
      var key = cast(at(args, 0), String);
      var val = cast(get(prices, key), Int);

      try {
        print_to(file, 0, "%$ :: %$\n", key, val);
      } catch (e in IOError) {
        println("Could not write to file - got %$", e);
      }

      return None;
    };

    /* Higher order functions */
    map(prices, write_pair);
  }

  delete(prices);
}
