#ifndef STR_VIEW_H
#define STR_VIEW_H

#include <stdbool.h>
#include <stdlib.h>
#define STR_FMT "%.*s"
#define STR_ARG(sv) (int)(sv).count, (sv).data

typedef struct {
  size_t count;
  const char *data;
} StringView;

#ifdef __cplusplus
extern "C" {
#endif

StringView sv_from_cstr(const char *str);
StringView sv_from_parts(const char *str, size_t count);

// Trimming (returns sliced view)
StringView sv_trim_left(StringView sv);
StringView sv_trim_right(StringView sv);
StringView sv_trim(StringView sv);

// Slicing by character / delimiter
StringView sv_cut_by_delim(StringView *sv, char delim);
bool sv_try_cut_by_delim(StringView *sv, char delim, StringView *chunk);
StringView sv_cut_left(StringView *sv, size_t n);
StringView sv_cut_right(StringView *sv, size_t n);

// Substring searching & cutting
int sv_index_of(StringView sv, char needle);
StringView sv_cut_with_sv(StringView *sv, StringView delim, bool ignore_case);

// Predicate trimming/slicing
StringView sv_take_left_pred(StringView sv, bool (*predicate)(char));
StringView sv_take_right_pred(StringView sv, bool (*predicate)(char));
StringView sv_cut_left_pred(StringView *sv, bool (*predicate)(char));

// Equality
bool sv_eq(StringView a, StringView b);
bool sv_eq_ignorecase(StringView a, StringView b);

#ifdef __cplusplus
}
#endif

// =============================================================================
// IMPLEMENTATION
// =============================================================================
#ifdef STR_VIEW_IMPLEMENTATION

#include <ctype.h>
#include <string.h>

StringView sv_from_cstr(const char *str) {
  if (!str)
    return (StringView){.count = 0, .data = NULL};
  return (StringView){.count = strlen(str), .data = str};
}

StringView sv_from_parts(const char *str, size_t count) {
  return (StringView){.count = count, .data = str};
}

StringView sv_trim_left(StringView sv) {
  size_t i = 0;
  while (i < sv.count && isspace((unsigned char)sv.data[i])) {
    i++;
  }
  return sv_from_parts(sv.data + i, sv.count - i);
}

StringView sv_trim_right(StringView sv) {
  size_t i = 0;
  while (i < sv.count && isspace((unsigned char)sv.data[sv.count - 1 - i])) {
    i++;
  }
  return sv_from_parts(sv.data, sv.count - i); // Fixed: was str.count
}

StringView sv_trim(StringView sv) { return sv_trim_right(sv_trim_left(sv)); }

StringView sv_cut_by_delim(StringView *sv, char delim) {
  size_t i = 0;
  while (i < sv->count && sv->data[i] != delim) {
    i++;
  }
  StringView result = sv_from_parts(sv->data, i);
  size_t advance = (i < sv->count) ? i + 1 : i;
  sv->data += advance;
  sv->count -= advance;
  return result;
}

bool sv_try_cut_by_delim(StringView *sv, char delim, StringView *chunk) {
  size_t i = 0;
  while (i < sv->count && sv->data[i] != delim) {
    i++;
  }
  StringView result = sv_from_parts(sv->data, i);
  if (i < sv->count) {
    sv->data += i + 1;
    sv->count -= i + 1;
    if (chunk)
      *chunk = result;
    return true;
  }
  sv->data += i;
  sv->count -= i;
  return false;
}

StringView sv_cut_left(StringView *sv, size_t n) {
  if (n > sv->count)
    n = sv->count;
  StringView result = sv_from_parts(sv->data, n);
  sv->data += n;
  sv->count -= n;
  return result;
}

StringView sv_cut_right(StringView *sv, size_t n) {
  if (n > sv->count)
    n = sv->count;
  StringView result = sv_from_parts(sv->data + sv->count - n, n);
  sv->count -= n;
  return result;
}

int sv_index_of(StringView sv, char needle) {
  for (size_t i = 0; i < sv.count; i++) {
    if (sv.data[i] == needle)
      return (int)i;
  }
  return -1;
}

bool sv_eq(StringView a, StringView b) {
  if (a.count != b.count)
    return false;
  return memcmp(a.data, b.data, a.count) == 0;
}

bool sv_eq_ignorecase(StringView a, StringView b) {
  if (a.count != b.count)
    return false;
  for (size_t i = 0; i < a.count; i++) {
    if (tolower((unsigned char)a.data[i]) !=
        tolower((unsigned char)b.data[i])) {
      return false;
    }
  }
  return true;
}

StringView sv_cut_with_sv(StringView *sv, StringView delim, bool ignore_case) {
  if (delim.count == 0 || sv->count < delim.count) {
    StringView res = *sv;
    sv->data += sv->count;
    sv->count = 0;
    return res;
  }

  size_t limit = sv->count - delim.count;
  size_t i = 0;
  bool found = false;

  while (i <= limit) {
    StringView window = sv_from_parts(sv->data + i, delim.count);
    if (ignore_case ? sv_eq_ignorecase(window, delim) : sv_eq(window, delim)) {
      found = true;
      break;
    }
    i++;
  }

  if (found) {
    StringView result = sv_from_parts(sv->data, i);
    sv->data += (i + delim.count);
    sv->count -= (i + delim.count);
    return result;
  }

  StringView result = *sv;
  sv->data += sv->count;
  sv->count = 0;
  return result;
}

StringView sv_take_left_pred(StringView sv, bool (*predicate)(char)) {
  size_t i = 0;
  while (i < sv.count && predicate(sv.data[i])) {
    i++;
  }
  return sv_from_parts(sv.data, i);
}

StringView sv_take_right_pred(StringView sv, bool (*predicate)(char)) {
  size_t i = sv.count;
  while (i > 0 && predicate(sv.data[i - 1])) {
    i--;
  }
  return sv_from_parts(sv.data + i, sv.count - i);
}

StringView sv_cut_left_pred(StringView *sv, bool (*predicate)(char)) {
  size_t i = 0;
  while (i < sv->count && predicate(sv->data[i])) {
    i++;
  }
  StringView result = sv_from_parts(sv->data, i);
  sv->data += i;
  sv->count -= i;
  return result;
}

#endif // STR_VIEW_IMPLEMENTATION
#endif // STR_VIEW_H
