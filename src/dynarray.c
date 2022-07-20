/*
 * This file is where you should implement your dynamic array.  It already
 * contains skeletons of the functions you need to implement (along with
 * documentation for each function).  Feel free to implement any additional
 * functions you might need.  Also, don't forget to include your name and
 * @oregonstate.edu email address below.
 *
 * Name: Hayden Johnson
 * Email: johnhayd@oregonstate.edu
 */

#include <stdio.h>
#include <stdlib.h>

#include "dynarray.h"

/*
 * This function should allocate and initialize a new, empty dynamic array and
 * return a pointer to it.  The array you allocate should have an initial
 * capacity of 2.
 */
struct dynarray dynarray_create() {
    struct dynarray da;
    da.data = (void**) malloc(sizeof(void*) * 2);
    da.size = -1;
    da.capacity = 2;

    return da;
}

/*
 * This function should free the memory associated with a dynamic array.  In
 * particular, while this function should up all memory used in the array
 * itself (i.e. the underlying `data` array), it should not free any memory
 * allocated to the pointer values stored in the array.  In other words, this
 * function does not need to *traverse* the array and free the individual
 * elements.  This is the responsibility of the caller.
 *
 * Params:
 *   da - the dynamic array to be destroyed.  May not be NULL.
 */
void dynarray_free(struct dynarray da) {
    free(da.data);
}

/*
 * This function should return the size of a given dynamic array (i.e. the
 * number of elements stored in it, not the capacity).
 */
int dynarray_size(struct dynarray da) {
    return da.size;
}

/*
 * This function should insert a new value to a given dynamic array.  For
 * simplicity, this function should only insert elements at the *end* of the
 * array.  In other words, it should always insert the new element immediately
 * after the current last element of the array.  If there is not enough space
 * in the dynamic array to store the element being inserted, this function
 * should double the size of the array.
 *
 * Params:
 *   da - the dynamic array into which to insert an element.  May not be NULL.
 *   val - the value to be inserted.  Note that this parameter has type void*,
 *     which means that a pointer of any type can be passed.
 */
void dynarray_insert(struct dynarray* da, void* val) {
    if (da == NULL)
    {
        fputs("[ERROR] Invalid argument in `dynarray_insert` call", stderr);
        exit(EXIT_FAILURE);
    }
    if (++da->size >= da->capacity)
    {
        void** temparr = (void**) realloc(da->data, da->capacity *= sizeof(void*) * 2);
        if (temparr == NULL)
        {
            fputs("[ERROR] realloc failed in `dynarray_insert` call", stderr);
            exit(EXIT_FAILURE);
        }
        da->data = temparr;
    }
    da->data[da->size] = val;
}

/*
 * This function should remove an element at a specified index from a dynamic
 * array.  All existing elements following the specified index should be moved
 * forward to fill in the gap left by the removed element.  In other words, if
 * the element at index i is removed, then the element at index i+1 should be
 * moved forward to index i, the element at index i+2 should be moved forward
 * to index i+1, the element at index i+3 should be moved forward to index i+2,
 * and so forth.
 *
 * Params:
 *   da - the dynamic array from which to remove an element.  May not be NULL.
 *   idx - the index of the element to be removed.  The value of `idx` must be
 *     between 0 (inclusive) and n (exclusive), where n is the number of
 *     elements stored in the array.
 */
void dynarray_remove(struct dynarray* da, int idx) {
    if (da == NULL || idx > da->size || idx < 0)
    {
        fputs("[ERROR] Invalid arguments in `dynarray_remove` call", stderr);
        exit(EXIT_FAILURE);
    }
    for (int i = idx + 1; i < da->size; ++i)
        da->data[i - 1] = da->data[i];
    da->size--;
    return;
}

/*
 * This function should return the value of an existing element a dynamic
 * array. Note that this value should be returned as type void*.
 *
 * Params:
 *   da - the dynamic array from which to get a value.  May not be NULL.
 *   idx - the index of the element whose value should be returned.  The value
 *     of `idx` must be between 0 (inclusive) and n (exclusive), where n is the
 *     number of elements stored in the array.
 */
void* dynarray_get(struct dynarray da, int idx) {
    if (idx > da.size || idx < 0)
    {
        fputs("[ERROR] Invalid arguments in `dynarray_get` call", stderr);
        exit(EXIT_FAILURE);
    }
    return da.data[idx];
}

/*
 * This function should update (i.e. overwrite) the value of an existing
 * element in a dynamic array.
 *
 * Params:
 *   da - the dynamic array in which to set a value.  May not be NULL.
 *   idx - the index of the element whose value should be updated.  The value
 *     of `idx` must be between 0 (inclusive) and n (exclusive), where n is the
 *     number of elements stored in the array.
 *   val - the new value to be set.  Note that this parameter has type void*,
 *     which means that a pointer of any type can be passed.
 */
void dynarray_set(struct dynarray da, int idx, void* val) {
    if (idx > da.size || idx < 0)
    {
        fputs("[ERROR] Invalid arguments in `dynarray_set` call", stderr);
        exit(EXIT_FAILURE);
    }
    da.data[idx] = val;
}

/*
 * This function returns a truncated array of all the data.
 * The last element in this array will be NULL.
 */
void** dynarray_raw(struct dynarray da)
{
    return da.data;
}
