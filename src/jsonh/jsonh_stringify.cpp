#include "blvckstd/jsonh.h"

/*
============================================================================
                                 Stringify                                  
============================================================================
*/

// Hoisted declarations
static void jsonh_stringify_obj(htable_t *obj, int indent, int indent_level, char **buf, size_t *buf_offs);
static void jsonh_stringify_arr(dynarr_t *arr, int indent, int indent_level, char **buf, size_t *buf_offs);

INLINED static char *jsonh_gen_indent(int indent)
{
  scptr char *buf = (char *) mman_alloc(sizeof(char), indent + 1, NULL);
  for (int i = 0; i < indent; i++)
    buf[i] = ' ';
  buf[indent] = 0;
  return (char *) mman_ref(buf);
}

static char *jsonh_escape_string(char *str)
{
  scptr char *res = (char *) mman_alloc(sizeof(char), 128, NULL);
  size_t res_ind = 0;

  for (char *c = str; *c; c++)
  {
    // Double buffer size when the end is reached
    mman_meta_t *res_meta = mman_fetch_meta(res);
    if (res_meta->num_blocks <= res_ind)
      mman_realloc((void **) &res, res_meta->block_size, res_meta->num_blocks * 2);

    // Escape characters by a leading backslash
    if (*c == '"' || *c == '\\')
      res[res_ind++] = '\\';

    res[res_ind++] = *c;
  }

  res[res_ind] = 0;
  return (char *) mman_ref(res);
}

/**
 * @brief Stringify a json-value based on it's type marker field
 * 
 * @param jv Jason value
 * @param indent Size of an indentation
 * @param indent_level Current level of indentation
 * @param buf String buffer to write into
 * @param buf_offs Current buffer offset, will be altered
 */
static void jsonh_stringify_value(jsonh_value_t *jv, int indent, int indent_level, char **buf, size_t *buf_offs)
{
  // Decide on type and call matching stfingifier
  switch (jv->type)
  {
    case JDTYPE_BOOL:
    strfmt(buf, buf_offs, "%s", *((bool *) jv->value) ? "true" : "false");
    return;

    case JDTYPE_INT:
    strfmt(buf, buf_offs, "%d", *((int *) jv->value));
    return;

    case JDTYPE_FLOAT:
    strfmt(buf, buf_offs, "%.7f", *((float *) jv->value));
    return;

    case JDTYPE_ARR:
    jsonh_stringify_arr((dynarr_t *) jv->value, indent, indent_level + 1, buf, buf_offs);
    return;

    case JDTYPE_OBJ:
    jsonh_stringify_obj((htable_t *) jv->value, indent, indent_level + 1, buf, buf_offs);
    return;

    case JDTYPE_NULL:
    strfmt(buf, buf_offs, "null");
    return;

    case JDTYPE_STR:
    {
      char *value = (char *) jv->value;
      scptr char *value_escaped = jsonh_escape_string(value);
      strfmt(buf, buf_offs, QUOTSTR, value_escaped);
      return;
    }
  }
}

/**
 * @brief Stringify an array to it's corresponding JSON value
 * 
 * Example value:
 * {
 *   "a": "b",
 *   "c": "d",
 *   "e": 55
 * }
 * 
 * @param arr Array to stringify
 * @param indent Size of an indentation
 * @param indent_level Current level of indentation
 * @param buf String buffer to write into
 * @param buf_offs Current buffer offset, will be altered
 */
static void jsonh_stringify_obj(htable_t *obj, int indent, int indent_level, char **buf, size_t *buf_offs)
{
  strfmt(buf, buf_offs, "{\n");
  scptr char *indent_str = jsonh_gen_indent(indent * indent_level);
  scptr char *indent_str_outer = jsonh_gen_indent(indent * u64_max(0, (uint64_t) indent_level - 1U));

  // Get all object keys
  scptr char **keys = NULL;
  htable_list_keys(obj, &keys);

  // Iterate object keys
  for (char **key = keys; *key; key++)
  {
    jsonh_value_t *jv = NULL;
    if (htable_fetch(obj, *key, (void **) &jv) != HTABLE_SUCCESS)
      continue;

    const char *sep = *(key + 1) == NULL ? "" : ",";
    strfmt(buf, buf_offs, "%s" QUOTSTR ": ", indent_str, *key);
    jsonh_stringify_value(jv, indent, indent_level, buf, buf_offs);
    strfmt(buf, buf_offs, "%s\n", sep);
  }

  strfmt(buf, buf_offs, "%s}", indent_str_outer);
}

/**
 * @brief Stringify an array to it's corresponding JSON value
 * 
 * Example value:
 * [
 *   1,
 *   2,
 *   3
 * ]
 * 
 * @param arr Array to stringify
 * @param indent Size of an indentation
 * @param indent_level Current level of indentation
 * @param buf String buffer to write into
 * @param buf_offs Current buffer offset, will be altered
 */
static void jsonh_stringify_arr(dynarr_t *arr, int indent, int indent_level, char **buf, size_t *buf_offs)
{
  strfmt(buf, buf_offs, "[\n");
  scptr char *indent_str = jsonh_gen_indent(indent * indent_level);
  scptr char *indent_str_outer = jsonh_gen_indent(indent * u64_max(0, (uint64_t) indent_level - 1U));

  // Get all array values
  scptr void **arr_vals = NULL;
  dynarr_as_array(arr, &arr_vals);
  
  // Iterate array values
  for (void **arr_v = arr_vals; *arr_v; arr_v++)
  {
    jsonh_value_t *jv = (jsonh_value_t *) *arr_v;
    const char *sep = *(arr_v + 1) == NULL ? "" : ",";
    strfmt(buf, buf_offs, "%s", indent_str);
    jsonh_stringify_value(jv, indent, indent_level, buf, buf_offs);
    strfmt(buf, buf_offs, "%s\n", sep);
  }

  strfmt(buf, buf_offs, "%s]", indent_str_outer);
}

char *jsonh_stringify(htable_t *jsonh, int indent, size_t initial_buf_size)
{
  scptr char *buf = (char *) mman_alloc(sizeof(char), initial_buf_size, NULL);
  size_t buf_offs = 0;

  jsonh_stringify_obj(jsonh, indent, 1, &buf, &buf_offs);
  strfmt(&buf, &buf_offs, "\n");

  return (char *) mman_ref(buf);
}