#ifndef STATUS_H
#define STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

enum trees_status {
    TREES_OK,
    TREES_NOT_FOUND,
    TREES_DUPLICATE_KEY,
    TREES_SYSTEM_ERROR
};

#ifdef __cplusplus
}
#endif

#endif // STATUS_H