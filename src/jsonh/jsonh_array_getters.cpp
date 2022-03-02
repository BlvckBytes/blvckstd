#include "blvckstd/jsonh.h"

/*
============================================================================
                              Array Getters                                 
============================================================================
*/

INLINED static jsonh_opres_t jsonh_get_arr_value(dynarr_t *array, int index, jsonh_datatype_t dt, void **output)
{
  if (index < 0 || index >= array->_array_size)
    return JOPRES_INVALID_INDEX;

  jsonh_value_t *value = (jsonh_value_t *) array->items[index];
  if (value->type != dt)
    return JOPRES_DTYPE_MISMATCH;

  if (output)
    *output = value->value;

  return JOPRES_SUCCESS;
}

jsonh_opres_t jsonh_get_arr_obj(dynarr_t *array, int index, htable_t *obj)
{
  return jsonh_get_arr_value(array, index, JDTYPE_OBJ, (void **) &obj);
}

jsonh_opres_t jsonh_get_arr_arr(dynarr_t *array, int index, dynarr_t *arr)
{
  return jsonh_get_arr_value(array, index, JDTYPE_ARR, (void **) &arr);
}

jsonh_opres_t jsonh_get_arr_str(dynarr_t *array, int index, char *str)
{
  return jsonh_get_arr_value(array, index, JDTYPE_STR, (void **) &str);
}

jsonh_opres_t jsonh_get_arr_int(dynarr_t *array, int index, int *num)
{
  int *val = NULL;
  jsonh_opres_t ret = jsonh_get_arr_value(array, index, JDTYPE_INT, (void **) &val);

  if (ret == JOPRES_SUCCESS)
    *num = *val;

  return ret;
}

jsonh_opres_t jsonh_get_arr_float(dynarr_t *array, int index, float *num)
{
  float *val = NULL;
  jsonh_opres_t ret = jsonh_get_arr_value(array, index, JDTYPE_FLOAT, (void **) &val);

  if (ret == JOPRES_SUCCESS)
    *num = *val;

  return ret;
}

jsonh_opres_t jsonh_get_arr_bool(dynarr_t *array, int index, bool *b)
{
  bool *val = NULL;
  jsonh_opres_t ret = jsonh_get_arr_value(array, index, JDTYPE_BOOL, (void **) &val);

  if (ret == JOPRES_SUCCESS)
    *b = *val;

  return ret;
}

jsonh_opres_t jsonh_get_arr_is_null(dynarr_t *array, int index, bool *is_null)
{
  jsonh_opres_t ret = jsonh_get_arr_value(array, index, JDTYPE_NULL, NULL);
  if (ret == JOPRES_INVALID_KEY)
    return ret;

  *is_null = ret != JOPRES_DTYPE_MISMATCH;
  return JOPRES_SUCCESS;
}