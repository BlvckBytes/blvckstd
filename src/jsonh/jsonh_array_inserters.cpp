#include "blvckstd/jsonh.h"

/*
============================================================================
                             Array Inserters                                
============================================================================
*/

INLINED static jsonh_opres_t jsonh_insert_value(dynarr_t *array, void *val, jsonh_datatype_t val_type)
{
  scptr jsonh_value_t* value = jsonh_value_make(val, val_type);
  if (dynarr_push(array, mman_ref(value), NULL) == DYNARR_SUCCESS)
    return JOPRES_SUCCESS;

  mman_dealloc(value);
  return JOPRES_SIZELIM_EXCEED;
}

jsonh_opres_t jsonh_insert_arr_obj(dynarr_t *array, void *obj)
{
  return jsonh_insert_value(array, obj, JDTYPE_OBJ);
}

jsonh_opres_t jsonh_insert_arr_obj_ref(dynarr_t *array, void *obj)
{
  jsonh_opres_t res = jsonh_insert_value(array, mman_ref(obj), JDTYPE_OBJ);
  if (res != JOPRES_SUCCESS)
    mman_dealloc(obj);
  return res;
}

jsonh_opres_t jsonh_insert_arr_arr(dynarr_t *array, void *arr)
{
  return jsonh_insert_value(array, arr, JDTYPE_ARR);
}

jsonh_opres_t jsonh_insert_arr_arr_ref(dynarr_t *array, void *arr)
{
  jsonh_opres_t res = jsonh_insert_value(array, mman_ref(arr), JDTYPE_ARR);
  if (res != JOPRES_SUCCESS)
    mman_dealloc(arr);
  return res;
}

jsonh_opres_t jsonh_insert_arr_str(dynarr_t *array, char *str)
{
  return jsonh_insert_value(array, (void *) str, JDTYPE_STR);
}

jsonh_opres_t jsonh_insert_arr_str_ref(dynarr_t *array, void *str)
{
  jsonh_opres_t res = jsonh_insert_value(array, mman_ref(str), JDTYPE_STR);
  if (res != JOPRES_SUCCESS)
    mman_dealloc(str);
  return res;
}

jsonh_opres_t jsonh_insert_arr_int(dynarr_t *array, int num)
{
  scptr int *numv = (int *) mman_alloc(sizeof(int), 1, NULL);
  *numv = num;

  jsonh_opres_t ret = jsonh_insert_value(array, mman_ref(numv), JDTYPE_INT);
  if (ret != JOPRES_SUCCESS)
    mman_dealloc(numv);

  return ret;
}

jsonh_opres_t jsonh_insert_arr_float(dynarr_t *array, float num)
{
  scptr float *numv = (float *) mman_alloc(sizeof(float), 1, NULL);
  *numv = num;

  jsonh_opres_t ret = jsonh_insert_value(array, mman_ref(numv), JDTYPE_FLOAT);
  if (ret != JOPRES_SUCCESS)
    mman_dealloc(numv);

  return ret;
}

jsonh_opres_t jsonh_insert_arr_bool(dynarr_t *array, bool b)
{
  scptr bool *boolv = (bool *) mman_alloc(sizeof(bool), 1, NULL);
  *boolv = b;

  jsonh_opres_t ret = jsonh_insert_value(array, mman_ref(boolv), JDTYPE_BOOL);
  if (ret != JOPRES_SUCCESS)
    mman_dealloc(boolv);

  return ret;
}

jsonh_opres_t jsonh_insert_arr_null(dynarr_t *array)
{
  return jsonh_insert_value(array, NULL, JDTYPE_NULL);
}
