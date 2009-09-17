#include <stdio.h>
#include "util.h"

// Hint for the compiler to optimize this function when not used 
#if DBG_LEVEL != 0
void util_pdbg(int msg, char *fmt, ...)
#else 
inline void util_pdbg(int msg, char *fmt, ...)
#endif 
{ 
  #if DBG_LEVEL == 0
  static char buf[DBG_MSG_LENGTH];

  if ( msg <= DBG_LEVEL ) {  
      vsprintf(buf, fmt, arg_ptr);
      fprintf(stderr, buf);
      fflush(stderr);
    }
   #endif
}
