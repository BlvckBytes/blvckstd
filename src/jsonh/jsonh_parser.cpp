#include "blvckstd/jsonh.h"

/*
============================================================================
                                  Parsing                                   
============================================================================
*/

jsonh_cursor_t *jsonh_cursor_make(const char *text)
{
  scptr jsonh_cursor_t *cursor = (jsonh_cursor_t *) mman_alloc(sizeof(jsonh_cursor_t), 1, NULL);

  // Set the text and calculate it's length
  // This gets saved for efficiency reasons, as strlen is quite slow
  cursor->text = text;
  cursor->text_length = strlen(text);

  // Start out at the first character
  cursor->text_index = 0;

  // Start out at the first line's first char
  cursor->prev_line_len = 0;
  cursor->char_index = 0;
  cursor->line_index = 0;

  return (jsonh_cursor_t *) mman_ref(cursor);
}

jsonh_char_t jsonh_cursor_getc(jsonh_cursor_t *cursor)
{
  jsonh_char_t ret = jsonh_cursor_peekc(cursor);

  // Linebreak occurred, update index trackers
  if (ret.c == '\n' && !ret.is_esc)
  {
    cursor->prev_line_len = cursor->char_index + 1;
    cursor->char_index = 0;
    cursor->line_index++;
  }

  // Character index should not be leading, like text_index is
  else if (cursor->text_index != 0)
    cursor->char_index++;

  // Advance position in text
  cursor->text_index++;
  return ret;
}

jsonh_char_t jsonh_cursor_peekc(jsonh_cursor_t *cursor)
{
  // EOF
  if (cursor->text_index == cursor->text_length)
    return (jsonh_char_t) { 0, false };

  // Fetch the current character
  char ret = cursor->text[cursor->text_index];

  // Check if this character is escaped
  bool is_esc = (
    cursor->text_index > 0                              // Not the first char (as it's unescapable)
    && cursor->text[cursor->text_index - 1] == '\\'     // And it's preceded by a backslash
  );

  return (jsonh_char_t) { ret, is_esc };
}

void jsonh_cursor_ungetc(jsonh_cursor_t *cursor)
{
  // Cannot go back any further
  if (cursor->text_index == 0)
    return;

  // Not at first char of line, just rewind char_index
  if (cursor->char_index > 0)
    cursor->char_index--;

  // At first char, wrap back to previous line_index
  else
  {
    cursor->char_index = cursor->prev_line_len;
    cursor->line_index--;
  }

  // Rewind text index marker
  cursor->text_index--;
}

INLINED static void jsonh_parse_err(jsonh_cursor_t *cursor, char **err, const char *fmt, ...)
{
  // No error buffer provided
  if (!err) return;

  // Generate error message from provided format and varargs
  va_list ap;
  va_start(ap, fmt);
  scptr char *errmsg = vstrfmt_direct(fmt, ap);
  va_end(ap);

  // Append prefix and write into error buffer
  *err = strfmt_direct(
    "(%ld:%ld) -> %s",

    // These values are zero-based internally but make
    // much more sense one-based in error-messages
    cursor->line_index + 1, cursor->char_index + 1,

    errmsg
  );
}

bool jsonh_parse_str(jsonh_cursor_t *cursor, char **err, char **out)
{
  jsonh_char_t curr;
  if ((curr = jsonh_cursor_getc(cursor)).c != '"')
  {
    jsonh_cursor_ungetc(cursor);
    jsonh_parse_err(cursor, err, "Expected >\"< but encountered >%c<", curr.c);
    return false;
  }

  // Save a copy of the string-starting cursor
  jsonh_cursor_t strstart_c = *cursor;

  // Allocate buffer
  scptr char *str = (char *) mman_alloc(sizeof(char), 128, NULL);
  size_t str_offs = 0;

  // Collect characters into a buffer
  while (true)
  {
    curr = jsonh_cursor_getc(cursor);

    // End of string reached
    if (curr.c == '"' && !curr.is_esc)
      break;

    // Non-printable characters need to be escaped inside of strings
    if (curr.c >= 1 && curr.c <= 31 && !curr.is_esc)
    {
      jsonh_cursor_ungetc(cursor);
      jsonh_parse_err(cursor, err, "Unescaped control sequence inside of string");
      return false;
    }

    // EOF before string has been closed
    if (!curr.c)
    {
      jsonh_parse_err(&strstart_c, err, "Unterminated string encountered");
      return false;
    }

    // Append to buffer
    strfmt(&str, &str_offs, "%c", curr.c);
  }

  *out = (char *) mman_ref(str);
  return true;
}

/*
  TODO: Not yet conform, missing the following:
  * e-notation (5e10, 5e-10)
*/
bool jsonh_parse_num(jsonh_cursor_t *cursor, char **err, double *out, bool *had_dot)
{
  bool has_dot = false, is_first = true;
  jsonh_char_t curr;

  // Collect digits and a possible dot
  scptr char *buf = (char *) mman_alloc(sizeof(char), 128, NULL);
  size_t buf_offs = 0;
  while ((curr = jsonh_cursor_getc(cursor)).c)
  {
    // Append to buffer
    strfmt(&buf, &buf_offs, "%c", curr.c);

    // Not a digit
    if (!(curr.c >= '0' && curr.c <= '9'))
    {
      // Numbers always start with digits or the optional negative sign
      if (is_first)
      {
        // Negative sign encountered, just add it to the buffer
        if (curr.c == '-')
        {
          is_first = false;
          continue;
        }

        jsonh_cursor_ungetc(cursor);
        jsonh_parse_err(cursor, err, "Expected a digit or a negative sign, but found >%c<", curr.c);
        return false;
      }

      // Found a dot which indicates floating point numbers
      if (curr.c == '.')
      {
        // Already has a dot
        if (has_dot)
        {
          jsonh_cursor_ungetc(cursor);
          jsonh_parse_err(cursor, err, "Numbers can only have one comma");
          return false;
        }

        // Now has a dot
        has_dot = true;
        is_first = false;
        continue;
      }

      // Done reading the number, unget the last char to be picked up by the next routine
      jsonh_cursor_ungetc(cursor);
      break;
    }

    is_first = false;
  }

  // Routine started out at EOF
  if (is_first && curr.c == 0)
  {
    jsonh_parse_err(cursor, err, "Expected a digit, but found EOF");
    return false;
  }

  // Write parsed number to output
  *out = atof(buf);
  if (had_dot) *had_dot = has_dot;
  return true;
}

bool jsonh_parse_literal(jsonh_cursor_t *cursor, char **err, jsonh_literal_t *out)
{
  jsonh_char_t curr;
  bool is_first = true;

  // Collect literal into buffer
  scptr char *buf = (char *) mman_alloc(sizeof(char), 128, NULL);
  size_t buf_offs = 0;
  jsonh_cursor_t first_cursor = *cursor;
  while ((curr = jsonh_cursor_getc(cursor)).c)
  {
    // Is a literal delimiter
    if (
      curr.c == ' '
      || curr.c == '}'
      || curr.c == ']'
      || curr.c == ','
      || (curr.c < ' ' || curr.c == 127) // Unprintables
    )
    {
      // Put back this delimiter to be picked up by the next routine
      jsonh_cursor_ungetc(cursor);
      break;
    }

    // Append to buffer
    strfmt(&buf, &buf_offs, "%c", curr.c);
    is_first = false;
  }

  // Routine started out at EOF
  if (is_first && curr.c == 0)
  {
    jsonh_parse_err(cursor, err, "Expected a literal, but found EOF");
    return false;
  }

  if (strcmp("true", buf) == 0)
  {
    *out = JLIT_TRUE;
    return true;
  }

  if (strcmp("false", buf) == 0)
  {
    *out = JLIT_FALSE;
    return true;
  }

  if (strcmp("null", buf) == 0)
  {
    *out = JLIT_NULL;
    return true;
  }

  jsonh_parse_err(&first_cursor, err, "Unknown literal " QUOTSTR, buf);
  return false;
}

bool jsonh_parse_value(jsonh_cursor_t *cursor, char **err, jsonh_value_t *out)
{
  // Peek at the first non-whitespace char
  jsonh_parse_eat_whitespace(cursor);
  jsonh_char_t fc = jsonh_cursor_peekc(cursor);

  // Number
  if ((fc.c >= '0' && fc.c <= '9') || fc.c == '-')
  {
    double num;
    bool had_dot;
    if (!jsonh_parse_num(cursor, err, &num, &had_dot))
      return false;

    // Had a dot and thus is a float
    if (had_dot)
    {
      out->type = JDTYPE_FLOAT;
      out->value = mman_alloc(sizeof(float), 1, NULL);
      *((float *) out->value) = num;
      return true;
    }

    // No dot, just a regular integer
    out->type = JDTYPE_INT;
    out->value = mman_alloc(sizeof(int), 1, NULL);
    *((int *) out->value) = num;
    return true;
  }

  // String
  if (fc.c == '"')
  {
    scptr char *str = NULL;
    if (!jsonh_parse_str(cursor, err, &str))
      return false;

    out->value = mman_ref(str);
    out->type = JDTYPE_STR;
    return true;
  }

  // Object
  if (fc.c == '{')
  {
    scptr htable_t *obj = NULL;
    if (!jsonh_parse_obj(cursor, err, &obj))
      return false;

    out->type = JDTYPE_OBJ;
    out->value = mman_ref(obj);
    return true;
  }

  // Array
  if (fc.c == '[')
  {
    scptr dynarr_t *arr = NULL;
    if (!jsonh_parse_arr(cursor, err, &arr))
      return false;

    out->type = JDTYPE_ARR;
    out->value = mman_ref(arr);
    return true;
  }

  // Only thing remaining: Literal
  jsonh_literal_t literal;
  jsonh_cursor_t first_cursor = *cursor;
  if (!jsonh_parse_literal(cursor, err, &literal))
    return false;

  switch(literal)
  {
    // Boolean value
    case JLIT_FALSE:
    case JLIT_TRUE:
    out->type = JDTYPE_BOOL;
    out->value = mman_alloc(sizeof(bool), 1, NULL);
    *((bool *) out->value) = literal == JLIT_TRUE;
    return true;

    // Null value
    case JLIT_NULL:
    out->type = JDTYPE_NULL;
    out->value = NULL;
    return true;

    default:
    jsonh_parse_err(&first_cursor, err, "Literal %s not yet implemented", jsonh_literal_name(literal));
    return false;
  }
}

bool jsonh_parse_arr(jsonh_cursor_t *cursor, char **err, dynarr_t **out)
{
  // Array start char
  jsonh_char_t curr;
  if ((curr = jsonh_cursor_getc(cursor)).c != '[')
  {
    jsonh_cursor_ungetc(cursor);
    jsonh_parse_err(cursor, err, "Expected array-begin but found >%c<", curr.c);
    return false;
  }

  jsonh_parse_eat_whitespace(cursor);

  // Parse values until the end of array is reached
  scptr dynarr_t *arr = dynarr_make(16, 1024, mman_dealloc_nr);
  while ((curr = jsonh_cursor_peekc(cursor)).c != ']')
  {
    jsonh_parse_eat_whitespace(cursor);

    // Parse a JSON value
    scptr jsonh_value_t *value = jsonh_value_make(NULL, JDTYPE_NULL);
    jsonh_cursor_t first_cursor = *cursor;
    if (!jsonh_parse_value(cursor, err, value))
      return false;
    
    dynarr_result_t res;
    if ((res = dynarr_push(arr, mman_ref(value), NULL)) != DYNARR_SUCCESS)
    {
      jsonh_parse_err(&first_cursor, err, "Could not push array value internally (%s)", dynarr_result_name(res));
      mman_dealloc(value);
      return false;
    }

    // Get the next non-whitespace char and check if it's a value separator
    // If not, stop reading
    jsonh_parse_eat_whitespace(cursor);
    if ((curr = jsonh_cursor_getc(cursor)).c != ',')
    {
      // Put back the non-separator for the next step to pick up
      jsonh_cursor_ungetc(cursor);
      break;
    }
    jsonh_parse_eat_whitespace(cursor);
  }

  jsonh_parse_eat_whitespace(cursor);

  // Array end char
  if ((curr = jsonh_cursor_getc(cursor)).c != ']')
  {
    jsonh_cursor_ungetc(cursor);
    jsonh_parse_err(cursor, err, "Expected array-end but found >%c<", curr.c);
    return false;
  }

  *out = (dynarr_t *) mman_ref(arr);
  return true;
}

bool jsonh_parse_obj(jsonh_cursor_t *cursor, char **err, htable_t **out)
{
  // Object start char
  jsonh_char_t curr;
  if ((curr = jsonh_cursor_getc(cursor)).c != '{')
  {
    jsonh_cursor_ungetc(cursor);
    jsonh_parse_err(cursor, err, "Expected object-begin but found >%c<", curr.c);
    return false;
  }

  jsonh_parse_eat_whitespace(cursor);

  // Parse values until the end of array is reached
  scptr htable_t *obj = htable_make(1024, mman_dealloc_nr);
  while ((curr = jsonh_cursor_peekc(cursor)).c != '}')
  {
    jsonh_parse_eat_whitespace(cursor);

    // Parse string key
    scptr char *key = NULL;
    if (!jsonh_parse_str(cursor, err, &key))
      return false;

    jsonh_parse_eat_whitespace(cursor);

    // Read k-v separator
    if ((curr = jsonh_cursor_getc(cursor)).c != ':')
    {
      jsonh_cursor_ungetc(cursor);
      jsonh_parse_err(cursor, err, "Expected >:< but found >%c<", curr.c);
      return false;
    }

    jsonh_parse_eat_whitespace(cursor);

    // Parse a JSON value
    scptr jsonh_value_t *value = jsonh_value_make(NULL, JDTYPE_NULL);
    jsonh_cursor_t first_cursor = *cursor;
    if (!jsonh_parse_value(cursor, err, value))
      return false;

    jsonh_parse_eat_whitespace(cursor);

    htable_result_t res;
    if ((res = htable_insert(obj, key, mman_ref(value))) != HTABLE_SUCCESS)
    {
      jsonh_parse_err(&first_cursor, err, "Could not push hashtable value internally (%s)", htable_result_name(res));
      mman_dealloc(value);
      return false;
    }

    // Get the next non-whitespace char and check if it's a value separator
    // If not, stop reading
    jsonh_parse_eat_whitespace(cursor);
    if ((curr = jsonh_cursor_getc(cursor)).c != ',')
    {
      // Put back the non-separator for the next step to pick up
      jsonh_cursor_ungetc(cursor);
      break;
    }
    jsonh_parse_eat_whitespace(cursor);
  }

  jsonh_parse_eat_whitespace(cursor);

  // Object end char
  if ((curr = jsonh_cursor_getc(cursor)).c != '}')
  {
    jsonh_cursor_ungetc(cursor);
    jsonh_parse_err(cursor, err, "Expected object-end but found >%c<", curr.c);
    return false;
  }

  *out = (htable_t *) mman_ref(obj);
  return true;
}

void jsonh_parse_eat_whitespace(jsonh_cursor_t *cursor)
{
  // Get chars until EOF or printable has been reached
  jsonh_char_t curr;
  while ((curr = jsonh_cursor_getc(cursor)).c > 0 && (curr.c <= 32 || curr.c == 127));

  // Unget a printable to be picked up by the next routine
  if (curr.c != 0)
    jsonh_cursor_ungetc(cursor);
}

htable_t *jsonh_parse(const char *json, char **err)
{
  scptr jsonh_cursor_t *cursor = jsonh_cursor_make(json);

  // JSON needs to have an object at root level
  scptr htable_t *res = NULL;
  if (!jsonh_parse_obj(cursor, err, &res))
    return NULL;

  return (htable_t *) mman_ref(res);
}