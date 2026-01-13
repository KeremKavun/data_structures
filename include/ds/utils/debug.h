#ifndef DEBUG_H
#define DEBUG_H

#ifdef NDEBUG
    #define CINFO
    #define CWARNING
    #define CERROR
    #define LIB_LVL
    #define PROJ_LVL
    #define USER_LVL
    #define LOG(lvl, status, fmt, ...)
    #define LOG_IF(cond, lvl, status, fmt, ...)  \
    do                                              \
    {                                               \
        if (cond)                                   \
            ;                                       \
    } while (0)        
#else
    #include <errno.h>
    #include <time.h>
    #include <limits.h>
    #include <string.h>
    #include <stdio.h>

    #define CINFO "\x1b[32m[INFO]\x1b[0m"
    #define CWARNING "\x1b[33m[WARNING]\x1b[0m"
    #define CERROR "\x1b[31m[ERROR]\x1b[0m"

    // The user might provide their own set of levels

    #define LIB_LVL 0
    #define PROJ_LVL 1
    #define USER_LVL 2
    #define GLOB_LVL INT_MAX // will be always greater than the current debug lvl in sane codes (INT_MAX to shut the gcc fuck up)

    #ifndef DBGLVL
        #define DBGLVL 0
    #endif // DBGLVL

    #define LOG(xxx_123_lvl, xxx_123_status, xxx_123_fmt, ...)      \
    do                                      \
    {                                       \
        if (xxx_123_lvl >= DBGLVL)                  \
        {                                   \
            char time_buff[21];             \
            time_t curr_time = time(NULL);  \
            strftime(time_buff, sizeof(time_buff), "%Y-%m-%d %H:%M:%S", localtime(&curr_time));                                         \
            if (errno == 0)                                                                                                             \
                fprintf(stderr, "\x1b[35m%s\x1b[0m " xxx_123_status " %s\x1b[34m@\x1b[36mfn-%s\x1b[34m@\x1b[36mln-%d\x1b[0m: " xxx_123_fmt "\n", time_buff, __FILE__, __func__, __LINE__, ##__VA_ARGS__);     \
            else                                                                                                                        \
                fprintf(stderr, "\x1b[35m%s\x1b[0m " xxx_123_status " %s\x1b[34m@\x1b[36mfn-%s\x1b[34m@\x1b[36mln-%d\x1b[0m: " xxx_123_fmt " (%s)\n", time_buff, __FILE__, __func__, __LINE__, ##__VA_ARGS__, strerror(errno)); \
            errno = 0;  \
        }   \
    } while (0)

    #define LOG_IF(xxx_123_cond, xxx_123_lvl, xxx_123_status, xxx_123_fmt, ...)  \
    do                                              \
    {                                               \
        if (xxx_123_cond)                                   \
            LOG(xxx_123_lvl, xxx_123_status, xxx_123_fmt, ##__VA_ARGS__);   \
    } while (0)                                     \

#endif

#endif // DEBUG_H