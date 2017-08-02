#ifndef IBNET_SYS_ASSERT_H
#define IBNET_SYS_ASSERT_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef ASSERT_MODULE
#define ASSERT_MODULE __FILE__
#endif

/**
 * Assert macro
 */
#define IBNET_ASSERT(expr) \
    assert(expr)

/**
 * Assert and if fails exit program
 */
#define IBNET_ASSERT_DIE(msg) \
    printf("%s", msg); \
    abort()

/**
 * Assert nullptr check
 */
#define IBNET_ASSERT_PTR(ptr) \
    assert(ptr != NULL)

/**
 * Assert if pointer is null
 */
#define IBNET_ASSERT_PTR_NULL(ptr) \
    assert(ptr == NULL)

#endif // IBNET_SYS_ASSERT_H
