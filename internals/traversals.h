#ifndef TRAVERSALS_H
#define TRAVERSALS_H

#ifdef __cplusplus
extern "C" {
#endif

enum traversal_order
{
    PREORDER,
    INORDER,
    POSTORDER
};

typedef enum traversal_order traversal_order_t;

#ifdef __cplusplus
}
#endif

#endif // TRAVERSALS_H