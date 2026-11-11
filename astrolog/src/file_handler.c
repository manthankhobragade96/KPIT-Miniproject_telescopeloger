#include "file_handler.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "observation.h"
#include "text_utils.h"
#include "validation.h"

/* Splits line in place at each delimiter; returns the field count (max + 1 if too many). */
static int splitFields(char *line, char *fields[], int maxFields)
{
    int count = 0;
    char *cursor = line;

    fields[count] = cursor;
    count++;

    while (*cursor != '\0')
    {
        if (*cursor == FIELD_DELIMITER)
        {
            *cursor = '\0';

            if (count >= maxFields)
            {
                return maxFields + 1;
            }

            fields[count] = cursor + 1;
            count++;
        }
        cursor++;
    }

    return count;
}

static bool parseIntField(const char *text, int *value)
{
    return parseInteger(text, value) == STATUS_SUCCESS;
}

Status_t parseRecordLine(const char *line, Observation_t *obs)
{
    if (line == NULL || obs == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    char buffer[LINE_BUFFER_SIZE];

    if (strlen(line) >= sizeof(buffer))
    {
        return STATUS_FILE_INVALID_DATA;
    }

    snprintf(buffer, sizeof(buffer), "%s", line);
    trimTrailingWhitespace(buffer);

    char *fields[RECORD_FIELD_COUNT];

    if (splitFields(buffer, fields, RECORD_FIELD_COUNT) != RECORD_FIELD_COUNT)
    {
        return STATUS_FILE_INVALID_DATA;
    }

    if (strlen(fields[1]) > MAX_TARGET_LEN || strlen(fields[9]) > MAX_NOTES_LEN)
    {
        return STATUS_FILE_INVALID_DATA;
    }

    Observation_t parsed;
    memset(&parsed, 0, sizeof(parsed));
    int typeValue = 0;

    if (!parseIntField(fields[0], &parsed.id) ||
        !parseIntField(fields[2], &typeValue) ||
        !parseIntField(fields[3], &parsed.date.day) ||
        !parseIntField(fields[4], &parsed.date.month) ||
        !parseIntField(fields[5], &parsed.date.year) ||
        !parseIntField(fields[6], &parsed.exposureSeconds) ||
        !parseIntField(fields[7], &parsed.frameCount) ||
        !parseIntField(fields[8], &parsed.seeing))
    {
        return STATUS_FILE_INVALID_DATA;
    }

    if (!isValidObjectType(typeValue))
    {
        return STATUS_FILE_INVALID_DATA;
    }

    parsed.type = (ObjectType_t)typeValue;
    snprintf(parsed.target, sizeof(parsed.target), "%s", fields[1]);
    snprintf(parsed.notes, sizeof(parsed.notes), "%s", fields[9]);

    if (validateObservation(&parsed) != STATUS_SUCCESS)
    {
        return STATUS_FILE_INVALID_DATA;
    }

    *obs = parsed;
    return STATUS_SUCCESS;
}

static Status_t writeAllRecords(FILE *file, const ObservationList_t *list)
{
    if (fprintf(file, "%s\n", DATA_FILE_HEADER) < 0)
    {
        return STATUS_FILE_WRITE_FAILED;
    }

    for (int i = 0; i < list->count; i++)
    {
        const Observation_t *obs = &list->items[i];

        /* The literal '|' in the format must match FIELD_DELIMITER. */
        int written = fprintf(file, "%d|%s|%d|%d|%d|%d|%d|%d|%d|%s\n",
                              obs->id, obs->target, (int)obs->type,
                              obs->date.day, obs->date.month, obs->date.year,
                              obs->exposureSeconds, obs->frameCount, obs->seeing,
                              obs->notes);

        if (written < 0)
        {
            return STATUS_FILE_WRITE_FAILED;
        }
    }

    if (fflush(file) != 0 || ferror(file))
    {
        return STATUS_FILE_WRITE_FAILED;
    }

    return STATUS_SUCCESS;
}

Status_t saveObservations(const ObservationList_t *list, const char *path)
{
    if (list == NULL || path == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    char tempPath[TEMP_PATH_SIZE];
    int length = snprintf(tempPath, sizeof(tempPath), "%s.tmp", path);

    if (length < 0 || (size_t)length >= sizeof(tempPath))
    {
        return STATUS_INVALID_ARGUMENT;
    }

    FILE *file = fopen(tempPath, "w");

    if (file == NULL)
    {
        return STATUS_FILE_OPEN_FAILED;
    }

    Status_t status = writeAllRecords(file, list);

    if (fclose(file) != 0 && status == STATUS_SUCCESS)
    {
        status = STATUS_FILE_WRITE_FAILED;
    }

    if (status == STATUS_SUCCESS && rename(tempPath, path) != 0)
    {
        status = STATUS_FILE_WRITE_FAILED;
    }

    if (status != STATUS_SUCCESS)
    {
        /* Best-effort cleanup of the temporary file; the original error is returned. */
        (void)remove(tempPath);
    }

    return status;
}

static void setErrorLine(int *errorLine, int lineNumber)
{
    if (errorLine != NULL)
    {
        *errorLine = lineNumber;
    }
}

static Status_t readAllRecords(FILE *file, ObservationList_t *list, int *errorLine)
{
    char line[LINE_BUFFER_SIZE];
    int lineNumber = 1;

    if (fgets(line, sizeof(line), file) == NULL)
    {
        return ferror(file) ? STATUS_FILE_READ_FAILED : STATUS_FILE_EMPTY;
    }

    trimTrailingWhitespace(line);

    if (strcmp(line, DATA_FILE_HEADER) != 0)
    {
        setErrorLine(errorLine, lineNumber);
        return STATUS_FILE_INVALID_DATA;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        lineNumber++;
        size_t length = strlen(line);

        if (length > 0 && line[length - 1] != '\n' && !feof(file))
        {
            setErrorLine(errorLine, lineNumber);
            return STATUS_FILE_INVALID_DATA;
        }

        if (isBlankString(line))
        {
            continue;
        }

        Observation_t obs;
        Status_t status = parseRecordLine(line, &obs);

        if (status == STATUS_SUCCESS)
        {
            status = addObservation(list, &obs);
        }

        if (status != STATUS_SUCCESS)
        {
            setErrorLine(errorLine, lineNumber);
            return (status == STATUS_CAPACITY_FULL) ? STATUS_CAPACITY_FULL
                                                    : STATUS_FILE_INVALID_DATA;
        }
    }

    return ferror(file) ? STATUS_FILE_READ_FAILED : STATUS_SUCCESS;
}

Status_t loadObservations(ObservationList_t *list, const char *path, int *errorLine)
{
    if (list == NULL || path == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    initObservationList(list);
    setErrorLine(errorLine, 0);

    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        return (errno == ENOENT) ? STATUS_FILE_NOT_FOUND : STATUS_FILE_OPEN_FAILED;
    }

    Status_t status = readAllRecords(file, list, errorLine);

    if (fclose(file) != 0 && status == STATUS_SUCCESS)
    {
        status = STATUS_FILE_READ_FAILED;
    }

    if (status != STATUS_SUCCESS)
    {
        initObservationList(list);
    }

    return status;
}
