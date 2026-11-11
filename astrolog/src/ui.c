#include "ui.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "observation.h"
#include "report.h"
#include "text_utils.h"
#include "validation.h"

#define CANCEL_KEYWORD "cancel"

static const int CHOICE_FIRST = 1;
static const int CHOICE_SECOND = 2;

/* ---------- low-level input ---------- */

static Status_t readLine(char *buffer, size_t size)
{
    if (buffer == NULL || size < 2)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (fgets(buffer, (int)size, stdin) == NULL)
    {
        return STATUS_INPUT_EOF;
    }

    size_t length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n')
    {
        trimTrailingWhitespace(buffer);
        return STATUS_SUCCESS;
    }

    if (feof(stdin))
    {
        return STATUS_SUCCESS;
    }

    /* Line did not fit: discard the rest of it. */
    int ch = getchar();

    while (ch != '\n' && ch != EOF)
    {
        ch = getchar();
    }

    return STATUS_INPUT_TOO_LONG;
}

/* Prompts until a line is read; "cancel" aborts the current operation. */
static Status_t promptLine(const char *prompt, char *buffer, size_t size)
{
    while (true)
    {
        printf("%s", prompt);
        Status_t status = readLine(buffer, size);

        if (status == STATUS_INPUT_TOO_LONG)
        {
            printf("Input is too long. Please try again.\n");
            continue;
        }

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        if (stringEqualsIgnoreCase(buffer, CANCEL_KEYWORD))
        {
            return STATUS_CANCELLED;
        }

        return STATUS_SUCCESS;
    }
}

static Status_t promptValidatedInt(const char *prompt, bool (*isValid)(int),
                                   int minValue, int maxValue, int *value)
{
    char buffer[INPUT_BUFFER_SIZE];

    while (true)
    {
        Status_t status = promptLine(prompt, buffer, sizeof(buffer));

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        int parsed = 0;

        if (parseInteger(buffer, &parsed) == STATUS_SUCCESS && isValid(parsed))
        {
            *value = parsed;
            return STATUS_SUCCESS;
        }

        printf("Invalid value. Enter a whole number from %d to %d.\n", minValue, maxValue);
    }
}

static Status_t promptDate(const char *prompt, Date_t *date)
{
    char buffer[INPUT_BUFFER_SIZE];

    while (true)
    {
        Status_t status = promptLine(prompt, buffer, sizeof(buffer));

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        if (parseDate(buffer, date) == STATUS_SUCCESS)
        {
            return STATUS_SUCCESS;
        }

        printf("Invalid date. Use DD-MM-YYYY with a real date (%d to %d).\n",
               MIN_YEAR, MAX_YEAR);
    }
}

/* ---------- validators used as callbacks ---------- */

static bool isValidTypeChoice(int choice)
{
    return isValidObjectType(choice - 1);
}

static bool isValidTwoWayChoice(int choice)
{
    return choice == CHOICE_FIRST || choice == CHOICE_SECOND;
}

/* ---------- field prompts ---------- */

static Status_t promptNewId(const ObservationList_t *list, int *id)
{
    char buffer[INPUT_BUFFER_SIZE];
    int suggested = getNextFreeId(list);

    while (true)
    {
        printf("Observation ID [%d-%d] (Enter = %d): ", MIN_ID, MAX_ID, suggested);
        Status_t status = readLine(buffer, sizeof(buffer));

        if (status == STATUS_INPUT_TOO_LONG)
        {
            printf("Input is too long. Please try again.\n");
            continue;
        }

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        if (stringEqualsIgnoreCase(buffer, CANCEL_KEYWORD))
        {
            return STATUS_CANCELLED;
        }

        if (isBlankString(buffer))
        {
            *id = suggested;
            return STATUS_SUCCESS;
        }

        int parsed = 0;

        if (parseInteger(buffer, &parsed) == STATUS_SUCCESS && isValidId(parsed))
        {
            *id = parsed;
            return STATUS_SUCCESS;
        }

        printf("Invalid ID. Enter a whole number from %d to %d.\n", MIN_ID, MAX_ID);
    }
}

static Status_t promptTarget(Observation_t *obs)
{
    char buffer[INPUT_BUFFER_SIZE];

    while (true)
    {
        Status_t status = promptLine("Target name: ", buffer, sizeof(buffer));

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        if (isValidTargetName(buffer))
        {
            copyText(obs->target, sizeof(obs->target), buffer);
            return STATUS_SUCCESS;
        }

        printf("Invalid target. Use 1 to %d characters, not blank, without '%c'.\n",
               MAX_TARGET_LEN, FIELD_DELIMITER);
    }
}

static Status_t promptObjectType(Observation_t *obs)
{
    printf("Object type:\n");

    for (int type = 0; type < OBJ_TYPE_COUNT; type++)
    {
        printf("  %d) %s\n", type + 1, objectTypeToString((ObjectType_t)type));
    }

    int choice = 0;
    Status_t status = promptValidatedInt("Choose type: ", isValidTypeChoice,
                                         1, OBJ_TYPE_COUNT, &choice);

    if (status == STATUS_SUCCESS)
    {
        obs->type = (ObjectType_t)(choice - 1);
    }

    return status;
}

static Status_t promptNotes(Observation_t *obs)
{
    char buffer[INPUT_BUFFER_SIZE];

    while (true)
    {
        Status_t status = promptLine("Notes (optional): ", buffer, sizeof(buffer));

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        if (isValidNotes(buffer))
        {
            copyText(obs->notes, sizeof(obs->notes), buffer);
            return STATUS_SUCCESS;
        }

        printf("Invalid notes. Use at most %d characters, without '%c'.\n",
               MAX_NOTES_LEN, FIELD_DELIMITER);
    }
}

/* Asks for every field except the ID. */
static Status_t promptObservationData(Observation_t *obs)
{
    Status_t status = promptTarget(obs);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = promptObjectType(obs);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = promptDate("Date (DD-MM-YYYY): ", &obs->date);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = promptValidatedInt("Exposure per frame in seconds: ", isValidExposure,
                                MIN_EXPOSURE_SECONDS, MAX_EXPOSURE_SECONDS,
                                &obs->exposureSeconds);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = promptValidatedInt("Number of frames: ", isValidFrameCount,
                                MIN_FRAME_COUNT, MAX_FRAME_COUNT, &obs->frameCount);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    status = promptValidatedInt("Seeing (1 = bad, 5 = excellent): ", isValidSeeing,
                                MIN_SEEING, MAX_SEEING, &obs->seeing);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    return promptNotes(obs);
}

/* ---------- output helpers ---------- */

static void reportOperationResult(Status_t status, const char *successMessage)
{
    if (status == STATUS_SUCCESS)
    {
        printf("%s\n", successMessage);
    }
    else
    {
        printf("%s\n", getStatusMessage(status));
    }
}

static void showSingleRecord(const ObservationList_t *list, int index)
{
    printObservationTable(stdout, list, &index, 1);
}

static Status_t promptExistingId(int *id)
{
    return promptValidatedInt("Observation ID: ", isValidId, MIN_ID, MAX_ID, id);
}

/* ---------- public interface ---------- */

const char *getStatusMessage(Status_t status)
{
    switch (status)
    {
    case STATUS_SUCCESS:
        return "Operation completed.";
    case STATUS_NULL_POINTER:
        return "Internal error: missing data.";
    case STATUS_INVALID_ARGUMENT:
        return "Invalid argument.";
    case STATUS_INVALID_ID:
        return "Invalid ID: must be a whole number from 1 to 999999.";
    case STATUS_INVALID_TARGET:
        return "Invalid target name.";
    case STATUS_INVALID_TYPE:
        return "Invalid object type.";
    case STATUS_INVALID_DATE:
        return "Invalid date.";
    case STATUS_INVALID_EXPOSURE:
        return "Invalid exposure time: must be 1 to 3600 seconds.";
    case STATUS_INVALID_FRAME_COUNT:
        return "Invalid frame count: must be 1 to 10000.";
    case STATUS_INVALID_SEEING:
        return "Invalid seeing rating: must be 1 to 5.";
    case STATUS_INVALID_NOTES:
        return "Invalid notes text.";
    case STATUS_DUPLICATE_ID:
        return "A record with this ID already exists.";
    case STATUS_NOT_FOUND:
        return "No record found.";
    case STATUS_EMPTY_COLLECTION:
        return "There are no records yet.";
    case STATUS_CAPACITY_FULL:
        return "The log is full. Delete a record before adding another.";
    case STATUS_FILE_NOT_FOUND:
        return "Data file not found.";
    case STATUS_FILE_OPEN_FAILED:
        return "Could not open the file.";
    case STATUS_FILE_EMPTY:
        return "The data file is empty.";
    case STATUS_FILE_INVALID_DATA:
        return "The data file contains invalid data.";
    case STATUS_FILE_READ_FAILED:
        return "Reading the file failed.";
    case STATUS_FILE_WRITE_FAILED:
        return "Writing the file failed.";
    case STATUS_INPUT_EOF:
        return "Input ended unexpectedly.";
    case STATUS_INPUT_TOO_LONG:
        return "Input is too long.";
    case STATUS_CANCELLED:
        return "Cancelled. No changes were made.";
    }

    return "Unknown error.";
}

void displayMenu(void)
{
    printf("\n===== AstroLog: Telescope Observation Log =====\n");
    printf(" 1. Add observation\n");
    printf(" 2. List all observations\n");
    printf(" 3. Search\n");
    printf(" 4. Edit observation\n");
    printf(" 5. Delete observation\n");
    printf(" 6. Filter\n");
    printf(" 7. Sort\n");
    printf(" 8. Summary report\n");
    printf(" 9. Export summary report to file\n");
    printf(" 0. Save and exit\n");
    printf("(Type 'cancel' at any prompt to abort an operation.)\n");
}

MenuChoice_t readMenuChoice(void)
{
    char buffer[INPUT_BUFFER_SIZE];

    printf("Enter choice: ");
    Status_t status = readLine(buffer, sizeof(buffer));

    if (status == STATUS_INPUT_EOF)
    {
        return MENU_EXIT;
    }

    int value = 0;

    if (status != STATUS_SUCCESS || parseInteger(buffer, &value) != STATUS_SUCCESS ||
        value < MENU_EXIT || value > MENU_EXPORT_REPORT)
    {
        return MENU_INVALID;
    }

    return (MenuChoice_t)value;
}

void handleAddObservation(ObservationList_t *list)
{
    if (list == NULL)
    {
        return;
    }

    Observation_t obs;
    memset(&obs, 0, sizeof(obs));

    Status_t status = promptNewId(list, &obs.id);

    if (status == STATUS_SUCCESS)
    {
        status = promptObservationData(&obs);
    }

    if (status == STATUS_SUCCESS)
    {
        status = addObservation(list, &obs);
    }

    reportOperationResult(status, "Observation added.");
}

void handleListObservations(const ObservationList_t *list)
{
    if (list == NULL)
    {
        return;
    }

    if (list->count == 0)
    {
        printf("%s\n", getStatusMessage(STATUS_EMPTY_COLLECTION));
        return;
    }

    printObservationTable(stdout, list, NULL, list->count);
    printf("%d record(s).\n", list->count);
}

void handleSearchObservations(const ObservationList_t *list)
{
    if (list == NULL)
    {
        return;
    }

    int choice = 0;
    printf("Search by: 1) ID  2) Target name\n");
    Status_t status = promptValidatedInt("Choose: ", isValidTwoWayChoice,
                                         CHOICE_FIRST, CHOICE_SECOND, &choice);

    if (status != STATUS_SUCCESS)
    {
        reportOperationResult(status, "");
        return;
    }

    if (choice == CHOICE_FIRST)
    {
        int id = 0;
        int index = 0;

        status = promptExistingId(&id);

        if (status == STATUS_SUCCESS)
        {
            status = findObservationIndexById(list, id, &index);
        }

        if (status == STATUS_SUCCESS)
        {
            showSingleRecord(list, index);
        }
        else
        {
            reportOperationResult(status, "");
        }

        return;
    }

    char keyword[INPUT_BUFFER_SIZE];
    int indices[MAX_OBSERVATIONS];

    status = promptLine("Target name contains: ", keyword, sizeof(keyword));

    if (status != STATUS_SUCCESS)
    {
        reportOperationResult(status, "");
        return;
    }

    int found = findObservationsByTarget(list, keyword, indices, MAX_OBSERVATIONS);

    if (found < 0)
    {
        printf("Search text cannot be empty.\n");
    }
    else if (found == 0)
    {
        printf("%s\n", getStatusMessage(STATUS_NOT_FOUND));
    }
    else
    {
        printObservationTable(stdout, list, indices, found);
        printf("%d match(es).\n", found);
    }
}

void handleEditObservation(ObservationList_t *list)
{
    if (list == NULL)
    {
        return;
    }

    int id = 0;
    int index = 0;
    Status_t status = promptExistingId(&id);

    if (status == STATUS_SUCCESS)
    {
        status = findObservationIndexById(list, id, &index);
    }

    if (status != STATUS_SUCCESS)
    {
        reportOperationResult(status, "");
        return;
    }

    printf("Current record:\n");
    showSingleRecord(list, index);
    printf("Enter the new values (the ID cannot be changed).\n");

    Observation_t updated;
    memset(&updated, 0, sizeof(updated));
    updated.id = id;

    status = promptObservationData(&updated);

    if (status == STATUS_SUCCESS)
    {
        status = updateObservation(list, id, &updated);
    }

    reportOperationResult(status, "Observation updated.");
}

void handleDeleteObservation(ObservationList_t *list)
{
    if (list == NULL)
    {
        return;
    }

    int id = 0;
    int index = 0;
    Status_t status = promptExistingId(&id);

    if (status == STATUS_SUCCESS)
    {
        status = findObservationIndexById(list, id, &index);
    }

    if (status != STATUS_SUCCESS)
    {
        reportOperationResult(status, "");
        return;
    }

    showSingleRecord(list, index);

    char answer[INPUT_BUFFER_SIZE];
    status = promptLine("Delete this record? (y/n): ", answer, sizeof(answer));

    if (status != STATUS_SUCCESS)
    {
        reportOperationResult(status, "");
        return;
    }

    if (!stringEqualsIgnoreCase(answer, "y") && !stringEqualsIgnoreCase(answer, "yes"))
    {
        printf("Deletion cancelled.\n");
        return;
    }

    reportOperationResult(deleteObservation(list, id), "Observation deleted.");
}

void handleFilterObservations(const ObservationList_t *list)
{
    if (list == NULL)
    {
        return;
    }

    int choice = 0;
    int indices[MAX_OBSERVATIONS];
    int found = 0;

    printf("Filter by: 1) Object type  2) Date range\n");
    Status_t status = promptValidatedInt("Choose: ", isValidTwoWayChoice,
                                         CHOICE_FIRST, CHOICE_SECOND, &choice);

    if (status == STATUS_SUCCESS && choice == CHOICE_FIRST)
    {
        Observation_t typeHolder;
        memset(&typeHolder, 0, sizeof(typeHolder));
        status = promptObjectType(&typeHolder);

        if (status == STATUS_SUCCESS)
        {
            found = filterByObjectType(list, typeHolder.type, indices, MAX_OBSERVATIONS);
        }
    }
    else if (status == STATUS_SUCCESS)
    {
        Date_t from;
        Date_t to;

        status = promptDate("From date (DD-MM-YYYY): ", &from);

        if (status == STATUS_SUCCESS)
        {
            status = promptDate("To date (DD-MM-YYYY): ", &to);
        }

        if (status == STATUS_SUCCESS)
        {
            found = filterByDateRange(list, from, to, indices, MAX_OBSERVATIONS);
        }
    }

    if (status != STATUS_SUCCESS)
    {
        reportOperationResult(status, "");
    }
    else if (found < 0)
    {
        printf("Invalid filter: the start date must not be after the end date.\n");
    }
    else if (found == 0)
    {
        printf("%s\n", getStatusMessage(STATUS_NOT_FOUND));
    }
    else
    {
        printObservationTable(stdout, list, indices, found);
        printf("%d record(s) match.\n", found);
    }
}

void handleSortObservations(ObservationList_t *list)
{
    if (list == NULL)
    {
        return;
    }

    if (list->count == 0)
    {
        printf("%s\n", getStatusMessage(STATUS_EMPTY_COLLECTION));
        return;
    }

    int choice = 0;
    printf("Sort by: 1) Date (oldest first)  2) Total integration (longest first)\n");
    Status_t status = promptValidatedInt("Choose: ", isValidTwoWayChoice,
                                         CHOICE_FIRST, CHOICE_SECOND, &choice);

    if (status == STATUS_SUCCESS)
    {
        SortKey_t key = (choice == CHOICE_FIRST) ? SORT_BY_DATE : SORT_BY_INTEGRATION;
        status = sortObservations(list, key);
    }

    if (status == STATUS_SUCCESS)
    {
        printObservationTable(stdout, list, NULL, list->count);
    }
    else
    {
        reportOperationResult(status, "");
    }
}

void handleSummaryReport(const ObservationList_t *list)
{
    Status_t status = printSummaryReport(stdout, list);

    if (status != STATUS_SUCCESS)
    {
        reportOperationResult(status, "");
    }
}

void handleExportReport(const ObservationList_t *list)
{
    Status_t status = exportSummaryReport(list, REPORT_FILE);

    if (status == STATUS_SUCCESS)
    {
        printf("Report written to %s\n", REPORT_FILE);
    }
    else
    {
        reportOperationResult(status, "");
    }
}
