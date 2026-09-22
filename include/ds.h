// #ifndef DS_H
// #define DS_H
//
// #include <assert.h>
// #include <string.h>
//
// #define da_append(da, x) \
//   do { \
//     static_assert( \
//         __builtin_types_compatible_p(typeof(*(da->array)), typeof(x)), \
//         "Type mismatch:Array type and item type not compatible"); \
//     da->count++; \
//     if (da->count >= da->cap) { \
//       da->cap == 0 ? da->cap *= 2; \
//       else da->array = realloc(da->array, (sizeof(*(da)->array)) * da->cap);
//       \
//     assert(da->array != NULL && "")
// }
// da->array[da->count - 1] = x;
// }
// while (0)
//
// #define MIN_CAP 8
// #define da_remove(da) \
//   do { \
//     assert(da->count > 0 && "There is no item in the array to remove"); \
//     da->count--; \
//     memset(&da->array[da->count], 0, sizeof(*(da)->array)); \
//     size_t new_cap = da->cap / 2; \
//     if (new_cap >= MIN_CAP && da->count <= new_cap / 2) { \
//       void *ptr = realloc(da->array, sizeof(*(da)->array) * new_cap); \
//       if (ptr != NULL) { \
//         da->array = ptr; \
//         da->cap = new_cap; \
//       } \
//     } \
//   } while (0)
//
// #endif // DS_H
