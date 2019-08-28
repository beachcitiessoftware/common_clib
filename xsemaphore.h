//
//  xSemaphore.h - x-platform semaphore for iOS & Android (POSIX)
//  AudioFetchSDK
//
//  Copyright © 2019 Beach Cities Software, LLC. All rights reserved.
//

#ifndef xsemaphore_h
#define xsemaphore_h

#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#if __APPLE__
#define __USE_DISPATCH__
#endif

#ifdef __USE_DISPATCH__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

/**
 Returns unix time in milliseconds
 (millis. since the Unix Epoch, Jan. 1, 1970 00:00:00)
 
 @return Returns the current unix time in milliseconds
 */
static inline uint64_t unix_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t unixts = ((uint64_t) (tv.tv_sec) * 1000) +
    ((uint64_t) (tv.tv_usec) / 1000);
    return unixts;
}

/**
 Unix timestamp in seconds
 
 @return Returns the current unix time converted to seconds
 */
static inline uint64_t unix_timestamp() {
    uint64_t unixTimeSeconds = unix_time() / 1000;
    return unixTimeSeconds;
}

/**
 x-platform (iOS/Android - POSIX) semaphore
 */
typedef struct xsemaphore {
#ifdef __USE_DISPATCH__
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
} xsemaphore;

/**
 Init xsemaphore
 
 @param s - The xsemaphore
 @param value - The value for the semaphore
 @return Returns the 0 on success, anything else on failure.
 */
static inline int xsem_init(struct xsemaphore *s, uint32_t value) {
#ifdef __USE_DISPATCH__
    dispatch_semaphore_t *sem = &s->sem;
    *sem = dispatch_semaphore_create(value);
    return 0;
#else
    return sem_init(&s->sem, 0, value);
#endif
}

/**
 Wait for semaphore
 
 @param s - The xsemaphore
 @param timeoutMS - Timeout in milliseconds, or >= 0 for infinite wait.
 @return Returns the result from the respective wait call.
 */
static inline long xsem_wait(struct xsemaphore *s, uint64_t timeoutMS = -1) {
#ifdef __USE_DISPATCH__
    timeoutMS = (timeoutMS > 0) ? timeoutMS : DISPATCH_TIME_FOREVER;
    dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(timeoutMS / 1000) * (double)NSEC_PER_SEC);
    return dispatch_semaphore_wait(s->sem, time);
#else
    int r;
    uint64_t now = unix_time();
    uint64_t end = (timeoutMS <= 0) ? now : (now + timeoutMS);
    
    do {
        now = unix_time();
        r = sem_wait(&s->sem);
        
        if (timeoutMS > 0 && now > end) {
            break;
        }
    } while (r == -1 && errno == EINTR);
    return (long)r;
#endif
}

/**
 Posts/Signals xsemaphore
 
 @param s - The xsemaphore
 @return Returns the result from the respective post/signal call (platform-specific).
 */
static inline long xsem_post(struct xsemaphore *s) {
#ifdef __USE_DISPATCH__
    return dispatch_semaphore_signal(s->sem);
#else
    return (long)sem_post(&s->sem);
#endif
}

/**
 Destroys/Releases the xsemaphore
 
 @param s - The xsemaphore
 @return Returns the 0 on success, anything else on failure.
 */
static inline int xsem_destroy(struct xsemaphore *s) {
#ifdef __USE_DISPATCH__
    dispatch_release(s->sem);
    return 0;
#else
    return sem_destroy(&s->sem);
#endif
}

#endif /* xsemaphore_h */
