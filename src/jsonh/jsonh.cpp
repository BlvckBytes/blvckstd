#include "blvckstd/jsonh.h"

ENUM_LUT_FULL_IMPL(jsonh_datatype, _EVALS_JSONH_DTYPE);
ENUM_LUT_FULL_IMPL(jsonh_literal, _EVALS_JSONH_LITERAL);
ENUM_LUT_FULL_IMPL(jsonh_opres, _EVALS_JSONH_OPRES);

/*
============================================================================
                                 Creation                                   
============================================================================
*/

htable_t *jsonh_make()
{
  scptr htable_t *res = htable_make(JSONH_ROOT_ITEM_CAP, mman_dealloc_nr);
  return (htable_t *) mman_ref(res);
}