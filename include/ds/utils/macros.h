#ifndef UTILS_MACROS_H
#define UTILS_MACROS_H

#define XCONCAT(a, b) a##b
#define CONCAT(a, b) XCONCAT(a, b)

#define UNIQUE_NAME(prefix) CONCAT(prefix, __LINE__)

#ifndef container_of
// Source - https://stackoverflow.com/q
// Posted by jaeyong, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-19, License - CC BY-SA 4.0

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)((char *)__mptr - offsetof(type,member));})

#define BINTREE_TAG_MASK ((uintptr_t)0x03)

#endif // container_of

#endif // UTILS_MACROS_H