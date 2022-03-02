#include "blvckstd/jsonh.h"

/*
============================================================================
                                 Getters                                    
============================================================================
*/

char *jsonh_getter_errstr(const char *used_key, jsonh_opres_t opres)
{
  switch(opres)
  {
    case JOPRES_INVALID_KEY:
    return strfmt_direct("Missing the property " QUOTSTR, used_key);

    case JOPRES_DTYPE_MISMATCH:
    return strfmt_direct("The datatype of the property " QUOTSTR " mismatches", used_key);

    default:
    return strfmt_direct("Unknown error");
  }
}

/**
 * @brief Get a value from a json object by type, checks that the key
 * exists and the type matches
 * 
 * @param jsonh Json object to fetch from
 * @param key Key of the target value
 * @param dt Expected datatype
 * @param output Output buffer
 * 
 * @return jsonh_opres_t Operation result
 */
INLINED static jsonh_opres_t jsonh_get_value(htable_t *jsonh, const char *key, jsonh_datatype_t dt, void **output)
{
  jsonh_value_t *value = NULL;
  if (htable_fetch(jsonh, key, (void **) &value) != HTABLE_SUCCESS)
    return JOPRES_INVALID_KEY;

  if (value->type != dt)
    return JOPRES_DTYPE_MISMATCH;

  if (output)
    *output = value->value;

  return JOPRES_SUCCESS;
}

jsonh_opres_t jsonh_get_obj(htable_t *jsonh, const char *key, htable_t **obj)
{
  return jsonh_get_value(jsonh, key, JDTYPE_OBJ, (void **) obj);
}

jsonh_opres_t jsonh_get_arr(htable_t *jsonh, const char *key, dynarr_t **arr)
{
  return jsonh_get_value(jsonh, key, JDTYPE_ARR, (void **) arr);
}

jsonh_opres_t jsonh_get_str(htable_t *jsonh, const char *key, char **str)
{
  return jsonh_get_value(jsonh, key, JDTYPE_STR, (void **) str);
}

jsonh_opres_t jsonh_get_int(htable_t *jsonh, const char *key, int *num)
{
  int *val = NULL;
  jsonh_opres_t ret = jsonh_get_value(jsonh, key, JDTYPE_INT, (void **) &val);

  if (ret == JOPRES_SUCCESS)
    *num = *val;

  return ret;
}

jsonh_opres_t jsonh_get_float(htable_t *jsonh, const char *key, float *num)
{
  float *val = NULL;
  jsonh_opres_t ret = jsonh_get_value(jsonh, key, JDTYPE_FLOAT, (void **) &val);

  if (ret == JOPRES_SUCCESS)
    *num = *val;

  return ret;
}

jsonh_opres_t jsonh_get_bool(htable_t *jsonh, const char *key, bool *b)
{
  bool *val = NULL;
  jsonh_opres_t ret = jsonh_get_value(jsonh, key, JDTYPE_BOOL, (void **) &val);

  if (ret == JOPRES_SUCCESS)
    *b = *val;

  return ret;
}

jsonh_opres_t jsonh_get_is_null(htable_t *jsonh, const char *key, bool *is_null)
{
  jsonh_opres_t ret = jsonh_get_value(jsonh, key, JDTYPE_NULL, NULL);
  if (ret == JOPRES_INVALID_KEY)
    return ret;

  *is_null = ret != JOPRES_DTYPE_MISMATCH;
  return JOPRES_SUCCESS;
}
