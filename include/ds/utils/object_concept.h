#ifndef OBJECT_CONCEPT_H
#define OBJECT_CONCEPT_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup OBJ_CONCEPT Object Concept
 * @ingroup UTILS
 * @brief Metadata for object lifecycle management (Initialization and Destruction).
 * 
 * @details
 * The Object Concept is a fundamental dependency used to handle generic data within 
 * containers. It bridges the gap between the container's raw memory management and 
 * the specific requirements of the user's data types.
 * * ### Lifecycle Logic
 * - **Initialization (`init`)**: Acts as a generalized copy constructor. For containers 
 * that own their data (e.g., @ref DYNARRAY), a valid `init` is mandatory. Arguments 
 * can be packed into the second `void *` parameter.
 * - **Deinitialization (`deinit`)**: Invoked during destruction. For data structures 
 * holding references only (e.g., @ref GRAPHS), this is primarily used during final 
 * cleanup.
 * * @{
 */

/**
 * @struct object_concept
 * @brief Stores function pointers for object initialization and destruction.
 * @note **Platform Support**: This signature assumes an x86-like environment where 
 * pointers are of uniform size. On other platforms, wrapper functions are required 
 * to ensure signature matching.
 */
struct object_concept
{
    /**
     * @brief Constructor/Copy initializer.
     * @param object Pointer to the memory slot to initialize.
     * @param args Pointer to source data or packed arguments.
     * @return Result code.
     * @retval 0 Success.
     * @retval Positive User-defined error code.
     * @retval Negative Reserved for internal codebase errors.
     */
    int (*init) (void *object, void *args); 

    /**
     * @brief Destructor/Cleanup.
     * @param object Pointer to the object to deinitialize.
     * @note May be NULL for POD types or if no specific cleanup is required.
     */
    void (*deinit) (void *object); 
};

/** @} */ // End of OBJ_CONCEPT group

#ifdef __cplusplus
}
#endif

#endif // OBJECT_CONCEPT_H