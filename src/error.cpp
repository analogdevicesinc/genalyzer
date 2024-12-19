#include "error.h"
#include <stdio.h>

void
error (const char *location, const char *msg)
{
  printf ("error at %s: %s\n", location, msg);
}