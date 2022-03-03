#include "blvckstd/dbglog.h"

void dbgerr(const char *fmt, ...)
{
  #ifdef DEBUG_ERR
  va_list ap;
  va_start(ap, fmt);
  scptr char *str = vstrfmt_direct(fmt, ap);
  va_end(ap);

  #ifdef DBGLOG_ARDUINO
  Serial.printf("ERR: %s\n", str);
  #else
  fprintf(stderr, "ERR: %s\n", str);
  #endif

  #endif
}

void dbginf(const char *fmt, ...)
{
  #ifdef DEBUG_INF
  va_list ap;
  va_start(ap, fmt);
  scptr char *str = vstrfmt_direct(fmt, ap);
  va_end(ap);

  #ifdef DBGLOG_ARDUINO
  Serial.printf("INF: %s\n", str);
  #else
  fprintf(stdout, "INF: %s\n", str);
  #endif

  #endif
}