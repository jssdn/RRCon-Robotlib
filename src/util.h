#ifndef __UTIL_H__
#define __UTIL_H__

//Xenomai
#include <native/mutex.h>
//--

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

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
void print_banner();

void __msleep(long t);
void __usleep(long long t);
void __nanosleep(long long t);

//TODO: UTIL_MUTEX_XXX_TAG for tag goto control during init/clean

#define UTIL_MUTEX_CREATE(DRIVER_NAME,MUTEX,DESC)	if((err = rt_mutex_create(MUTEX, DESC)) < 0 ){ \
							    util_pdbg(DBG_WARN, "%s: Error rt_mutex_create: %d\n",DRIVER_NAME, err);\
							    return err;\
							} 
						      
	
#define UTIL_MUTEX_DELETE(DRIVER_NAME,MUTEX)	if((err = rt_mutex_delete(MUTEX)) < 0 ){ \
						    util_pdbg(DBG_WARN, "%s: Mutex cannot be deleted: %d\n",DRIVER_NAME, err);\
						    return err;\
						}
						       
#define UTIL_MUTEX_ACQUIRE(DRIVER_NAME,MUTEX,TIME)	if((err = rt_mutex_acquire(MUTEX, TIME)) < 0 ){ \
							    util_pdbg(DBG_WARN, "%s: Couldn't acquire mutex: %d\n", DRIVER_NAME,err); \
							    return err;\
							}

#define UTIL_MUTEX_RELEASE(DRIVER_NAME,MUTEX)	if( (err = rt_mutex_release(MUTEX)) < 0 ){ \
						    util_pdbg(DBG_WARN, "%s: Couldn't release mutex: %d\n", DRIVER_NAME, err); \
						    return err; \
						 }

#endif 
