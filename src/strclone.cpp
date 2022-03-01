#include "blvckstd/strclone.h"

char *strclone_s(const char *origin, size_t max_len)
{
  size_t len = strlen(origin);

  // Create a "carbon copy"
  scptr char *clone = (char *) mman_alloc(sizeof(char), len + 1, NULL);
  size_t i;
  for (i = 0; i < u64_min(len, max_len); i++)
    clone[i] = origin[i];
  clone[i] = 0;

  return (char *) mman_ref(clone);
}

char *strclone(const char *origin)
{
  return strfmt_direct("%s", origin);
}