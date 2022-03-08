#include "blvckstd/atomanip.h"

size_t atomic_add(volatile size_t *target, const size_t value)
{
  #ifdef ESP8266
  *target += value;
  return *target;
  #else
  size_t old, n;

  // Try to compare and swap atomically until succeeded
  do {
    old = *target;
    n = old + value;
  } while (
    !__sync_bool_compare_and_swap(target, old, n)
  );

  // Return the new value
  return n;
  #endif
}

size_t atomic_increment(volatile size_t *target)
{
  return atomic_add(target, 1);
}

size_t atomic_decrement(volatile size_t *target)
{
  return atomic_add(target, -1);
}