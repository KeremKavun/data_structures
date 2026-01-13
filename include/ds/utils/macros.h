#ifndef UTILS_MACROS_H
#define UTILS_MACROS_H

#ifndef container_of
// Source - https://stackoverflow.com/q
// Posted by jaeyong, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-19, License - CC BY-SA 4.0

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)((char *)__mptr - offsetof(type,member));})

// Source - https://stackoverflow.com/a/30131043
// Posted by Shafik Yaghmour, modified by community. See post 'Timeline' for change history
// Retrieved 2026-01-03, License - CC BY-SA 3.0

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))       

#endif // container_of

#endif // UTILS_MACROS_H