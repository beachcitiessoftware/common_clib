//
//  xSemaphore.h
//  AudioFetchSDK
//
//  Copyright © 2019 Beach Cities Software, LLC. All rights reserved.
//

#ifndef xsemaphore_h
#define xsemaphore_h
#include "SSDP.h"
#include <errno.h>

#if __APPLE__
#define __DISPATCH__
#endif

#ifdef __DISPATCH__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

typedef struct xsemaphore {
#ifdef __DISPATCH__
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
} xsemaphore;

static inline int xsem_init(struct xsemaphore *s, uint32_t value) {
#ifdef __DISPATCH__
    dispatch_semaphore_t *sem = &s->sem;
    *sem = dispatch_semaphore_create(value);
    return 0;
#else
    return sem_init(&s->sem, 0, value);
#endif
}

static inline long xsem_wait(struct xsemaphore *s, uint64_t timeout = -1) {
#ifdef __DISPATCH__
    timeout = (timeout > 0) ? timeout : DISPATCH_TIME_FOREVER;
    dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, (int64_t)timeout * (double)NSEC_PER_SEC);
    return dispatch_semaphore_wait(s->sem, time);
#else
    int r;
    _unixTimestamp now = SSDP::getTimestampSeconds();
    _unixTimestamp end = (timeout <= 0) ? now : (now + timeout);
    
    do {
        now = SSDP::getTimestampSeconds();
        r = sem_wait(&s->sem);
        
        if (timeout > 0 && now > end) {
            break;
        }
    } while (r == -1 && errno == EINTR);
    return (long)r;
#endif
}

static inline long xsem_post(struct xsemaphore *s) {
#ifdef __DISPATCH__
    return dispatch_semaphore_signal(s->sem);
#else
    return (long)sem_post(&s->sem);
#endif
}

static inline int xsem_destroy(struct xsemaphore *s) {
#ifdef __DISPATCH__
    dispatch_release(s->sem);
    return 0;
#else
    return sem_destroy(&s->sem);
#endif
}

#endif /* xsemaphore_h */
