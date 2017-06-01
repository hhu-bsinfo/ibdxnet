#ifndef IBNET_SYS_ASSERT_H
#define IBNET_SYS_ASSERT_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef ASSERT_MODULE
#define ASSERT_MODULE __FILE__
#endif

#define IBNET_ASSERT(expr) \
    assert(expr)

#define IBNET_ASSERT_DIE(msg) \
    printf("%s", msg); \
    abort()

#define IBNET_ASSERT_PTR(ptr) \
    assert(ptr != NULL)

#define IBNET_ASSERT_PTR_NULL(ptr) \
    assert(ptr == NULL)

#endif // IBNET_SYS_ASSERT_H
