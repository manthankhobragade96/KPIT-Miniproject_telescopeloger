#include "text_utils.h"

#include <ctype.h>
#include <string.h>

bool stringEqualsIgnoreCase(const char *first, const char *second)
{
    if (first == NULL || second == NULL)
    {
        return false;
    }

    while (*first != '\0' && *second != '\0')
    {
        if (tolower((unsigned char)*first) != tolower((unsigned char)*second))
        {
            return false;
        }
        first++;
        second++;
    }

    return *first == *second;
}

bool stringContainsIgnoreCase(const char *text, const char *keyword)
{
    if (text == NULL || keyword == NULL || keyword[0] == '\0')
    {
        return false;
    }

    size_t textLength = strlen(text);
    size_t keywordLength = strlen(keyword);

    if (keywordLength > textLength)
    {
        return false;
    }

    for (size_t start = 0; start + keywordLength <= textLength; start++)
    {
        size_t offset = 0;

        while (offset < keywordLength &&
               tolower((unsigned char)text[start + offset]) ==
                   tolower((unsigned char)keyword[offset]))
        {
            offset++;
        }

        if (offset == keywordLength)
        {
            return true;
        }
    }

    return false;
}

void trimTrailingWhitespace(char *text)
{
    if (text == NULL)
    {
        return;
    }

    size_t length = strlen(text);

    while (length > 0 && isspace((unsigned char)text[length - 1]))
    {
        length--;
        text[length] = '\0';
    }
}

bool isBlankString(const char *text)
{
    if (text == NULL)
    {
        return true;
    }

    while (*text != '\0')
    {
        if (!isspace((unsigned char)*text))
        {
            return false;
        }
        text++;
    }

    return true;
}

void copyText(char *destination, size_t size, const char *source)
{
    if (destination == NULL || size == 0)
    {
        return;
    }

    size_t length = (source != NULL) ? strlen(source) : 0;

    if (length >= size)
    {
        length = size - 1;
    }

    if (length > 0)
    {
        memcpy(destination, source, length);
    }

    destination[length] = '\0';
}
