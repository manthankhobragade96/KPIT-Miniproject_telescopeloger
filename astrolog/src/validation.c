#include "validation.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "text_utils.h"

#define MONTHS_PER_YEAR 12
#define FEBRUARY 2
#define LAST_CONTROL_CHARACTER 31
#define DELETE_CHARACTER 127

static bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int daysInMonth(int month, int year)
{
    static const int DAYS[MONTHS_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == FEBRUARY && isLeapYear(year))
    {
        return DAYS[month - 1] + 1;
    }

    return DAYS[month - 1];
}

/* Control characters and the file delimiter are not allowed inside text fields. */
static bool containsForbiddenCharacter(const char *text)
{
    for (; *text != '\0'; text++)
    {
        unsigned char ch = (unsigned char)*text;

        if (ch <= LAST_CONTROL_CHARACTER || ch == DELETE_CHARACTER || ch == FIELD_DELIMITER)
        {
            return true;
        }
    }

    return false;
}

bool isValidId(int id)
{
    return id >= MIN_ID && id <= MAX_ID;
}

bool isValidTargetName(const char *name)
{
    if (name == NULL)
    {
        return false;
    }

    size_t length = strlen(name);

    if (length == 0 || length > MAX_TARGET_LEN)
    {
        return false;
    }

    return !isBlankString(name) && !containsForbiddenCharacter(name);
}

bool isValidObjectType(int typeValue)
{
    return typeValue >= 0 && typeValue < OBJ_TYPE_COUNT;
}

bool isValidDate(Date_t date)
{
    if (date.year < MIN_YEAR || date.year > MAX_YEAR)
    {
        return false;
    }

    if (date.month < 1 || date.month > MONTHS_PER_YEAR)
    {
        return false;
    }

    return date.day >= 1 && date.day <= daysInMonth(date.month, date.year);
}

bool isValidExposure(int seconds)
{
    return seconds >= MIN_EXPOSURE_SECONDS && seconds <= MAX_EXPOSURE_SECONDS;
}

bool isValidFrameCount(int frames)
{
    return frames >= MIN_FRAME_COUNT && frames <= MAX_FRAME_COUNT;
}

bool isValidSeeing(int seeing)
{
    return seeing >= MIN_SEEING && seeing <= MAX_SEEING;
}

bool isValidNotes(const char *notes)
{
    if (notes == NULL)
    {
        return false;
    }

    return strlen(notes) <= MAX_NOTES_LEN && !containsForbiddenCharacter(notes);
}

Status_t validateObservation(const Observation_t *obs)
{
    if (obs == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (!isValidId(obs->id))
    {
        return STATUS_INVALID_ID;
    }

    if (!isValidTargetName(obs->target))
    {
        return STATUS_INVALID_TARGET;
    }

    if (!isValidObjectType((int)obs->type))
    {
        return STATUS_INVALID_TYPE;
    }

    if (!isValidDate(obs->date))
    {
        return STATUS_INVALID_DATE;
    }

    if (!isValidExposure(obs->exposureSeconds))
    {
        return STATUS_INVALID_EXPOSURE;
    }

    if (!isValidFrameCount(obs->frameCount))
    {
        return STATUS_INVALID_FRAME_COUNT;
    }

    if (!isValidSeeing(obs->seeing))
    {
        return STATUS_INVALID_SEEING;
    }

    if (!isValidNotes(obs->notes))
    {
        return STATUS_INVALID_NOTES;
    }

    return STATUS_SUCCESS;
}

Status_t parseDate(const char *text, Date_t *date)
{
    if (text == NULL || date == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    int day = 0;
    int month = 0;
    int year = 0;
    int consumed = 0;

    if (sscanf(text, "%d-%d-%d%n", &day, &month, &year, &consumed) != 3)
    {
        return STATUS_INVALID_DATE;
    }

    if (!isBlankString(text + consumed))
    {
        return STATUS_INVALID_DATE;
    }

    Date_t parsed = {day, month, year};

    if (!isValidDate(parsed))
    {
        return STATUS_INVALID_DATE;
    }

    *date = parsed;
    return STATUS_SUCCESS;
}

Status_t parseInteger(const char *text, int *value)
{
    if (text == NULL || value == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (isBlankString(text))
    {
        return STATUS_INVALID_ARGUMENT;
    }

    char *end = NULL;
    errno = 0;
    long parsed = strtol(text, &end, 10);

    if (errno == ERANGE || parsed > INT_MAX || parsed < INT_MIN)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (end == text || !isBlankString(end))
    {
        return STATUS_INVALID_ARGUMENT;
    }

    *value = (int)parsed;
    return STATUS_SUCCESS;
}
