/* Use when compiling with BZ_NO_STDIO */

#include <assert.h>
#include <stdbool.h>

void bz_internal_error(int errcode __attribute__((unused)))
{
  assert(false);
}
