#ifndef _TIME_H_
#define _TIME_H_

/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: ANSI-C header file time.h
    Lang: english
*/
#include <aros/systypes.h>
#include <sys/_posix.h>

#ifdef	_AROS_TIME_T_
typedef _AROS_TIME_T_	    time_t;
#undef	_AROS_TIME_T_
#endif

#ifdef	_AROS_CLOCK_T_
typedef _AROS_CLOCK_T_	    clock_t;
#undef	_AROS_CLOCK_T_
#endif

#ifdef	_AROS_SIZE_T_
typedef _AROS_SIZE_T_	    size_t;
#undef	_AROS_SIZE_T_
#endif

#ifndef NULL
#   define NULL	    0
#endif

/* XXX: This is supposed to be 1000000 on SUSv2 platforms apparently */
#define CLOCKS_PER_SEC 50

struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

#if !defined(_ANSI_SOURCE) && defined(_P1003_1B_VISIBLE)

#ifdef	_AROS_TIMER_T_
typedef	_AROS_TIMER_T_	    timer_t;
#undef	_AROS_TIMER_T_
#endif

#ifdef	_AROS_CLOCKID_T_
typedef _AROS_CLOCKID_T_    clockid_t;
#undef	_AROS_CLOCKID_T_
#endif

struct timespec
{
    time_t		tv_sec;		/* seconds */
    long		tv_nsec;	/* nanoseconds */
};

struct itimerspec
{
    struct timespec	it_interval;	/* timer period */
    struct timespec	it_value;	/* timer expiration */
};

#define CLOCK_REALTIME		0
#define TIMER_ABSTIME		1

/* time.h shouldn't include signal.h */
struct sigevent;

#endif /* !_ANSI_SOURCE && _P1003_1B_VISIBLE */

#if !defined _CLIB_KERNEL_ && !defined _CLIB_LIBRARY_
    extern int       daylight;
    extern long int  timezone;
    extern char     *tzname[];
#else
#   include <libraries/arosc.h>
#endif

__BEGIN_DECLS
char      *asctime(const struct tm *);
clock_t    clock(void);
char      *ctime(const time_t *);
double     difftime(time_t, time_t);
struct tm *gmtime(const time_t *);
struct tm *localtime(const time_t *);
time_t     mktime(struct tm *);
size_t     strftime(char *, size_t, const char *, const struct tm *);
time_t     time(time_t *);

#if !defined(_ANSI_SOURCE)
void       tzset(void);
#endif

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
char      *asctime_r(const struct tm *, char *);
char      *ctime_r(const time_t *, char *);
struct tm *getdate(const char *);
struct tm *gmtime_r(const time_t *, struct tm *);
struct tm *localtime_r(const time_t *, struct tm *);
char      *strptime(const char *, const char *, struct tm *);
#endif /* !_ANSI_SOURCE && !_POSIX_SOURCE */

#if !defined(_ANSI_SOURCE) && defined(_P1003_1B_VISIBLE)
int        clock_getres(clockid_t, struct timespec *);
int        clock_gettime(clockid_t, struct timespec *);
int        clock_settime(clockid_t, const struct timespec *);
int        nanosleep(const struct timespec *, struct timespec *);
int        timer_create(clockid_t, struct sigevent *, timer_t *);
int        timer_delete(timer_t);
int        timer_gettime(timer_t, struct itimerspec *);
int        timer_getoverrun(timer_t);
int        timer_settime(timer_t, int, const struct itimerspec *,
               struct itimerspec *);
#endif /* !_ANSI_SOURCE && _P1003_1B_VISIBLE */

__END_DECLS

#endif /* _TIME_H_ */
