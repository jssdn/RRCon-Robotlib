#ifndef __UTIL_H__
#define __UTIL_H__

/*   --- Debug Loglevel ---
     DBG_NONE 0     silent 
     DBG_CRIT 2     critical error for application to exit 
     DBG_WARN 3     error that cause malfunction on the application if not handled
     DBG_INFO 4     useful information for tracking the program behaviour 
     DBG_DEBG 4     debug information

*/

#define DBG_MSG_LENGTH 80
#define DBG_NONE 0
#define DBG_CRIT 2
#define DBG_WARN 3
#define DBG_INFO 4
#define DBG_DEBG 5

// defines the current level of debugging. But should be somewhere else
#ifndef DBG_LEVEL
    #define DBG_LEVEL 5 
#endif

#if DBG_LEVEL != 0
void util_pdbg(int msg, char *fmt, ...);
#else 
inline void util_pdbg(int msg, char *fmt, ...);
#endif 

void print_timestamp();

#endif 
