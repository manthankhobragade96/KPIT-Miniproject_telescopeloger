#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns true when both strings are non-NULL and equal ignoring letter case. */
bool stringEqualsIgnoreCase(const char *first, const char *second);

/* Returns true when keyword (non-empty) occurs inside text, ignoring letter case. */
bool stringContainsIgnoreCase(const char *text, const char *keyword);

/* Removes trailing spaces, tabs and line-break characters in place. */
void trimTrailingWhitespace(char *text);

/* Returns true when text is NULL, empty or contains only whitespace. */
bool isBlankString(const char *text);

/* Copies source into destination, truncating if needed; the result is always terminated. */
void copyText(char *destination, size_t size, const char *source);

#ifdef __cplusplus
}
#endif

#endif /* TEXT_UTILS_H */
