/*
 * main.c
 * AstroLog: telescope observation log manager.
 * Only high-level control lives here: load data, run the menu loop, save, exit.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "file_handler.h"
#include "observation.h"
#include "ui.h"

static bool loadStoredData(ObservationList_t *list, const char *path)
{
    int errorLine = 0;
    Status_t status = loadObservations(list, path, &errorLine);

    if (status == STATUS_SUCCESS)
    {
        printf("Loaded %d record(s) from %s\n", list->count, path);
        return true;
    }

    if (status == STATUS_FILE_NOT_FOUND || status == STATUS_FILE_EMPTY)
    {
        printf("No existing data in %s. Starting with an empty log.\n", path);
        return true;
    }

    fprintf(stderr, "Cannot load %s: %s", path, getStatusMessage(status));

    if (errorLine > 0)
    {
        fprintf(stderr, " (line %d)", errorLine);
    }

    fprintf(stderr, "\nFix or remove the file and start again. Nothing was changed.\n");
    return false;
}

int main(int argc, char *argv[])
{
    const char *dataPath = (argc > 1) ? argv[1] : DEFAULT_DATA_FILE;
    ObservationList_t list;

    initObservationList(&list);

    if (!loadStoredData(&list, dataPath))
    {
        return EXIT_FAILURE;
    }

    bool running = true;

    while (running)
    {
        displayMenu();

        switch (readMenuChoice())
        {
        case MENU_ADD:
            handleAddObservation(&list);
            break;
        case MENU_LIST:
            handleListObservations(&list);
            break;
        case MENU_SEARCH:
            handleSearchObservations(&list);
            break;
        case MENU_EDIT:
            handleEditObservation(&list);
            break;
        case MENU_DELETE:
            handleDeleteObservation(&list);
            break;
        case MENU_FILTER:
            handleFilterObservations(&list);
            break;
        case MENU_SORT:
            handleSortObservations(&list);
            break;
        case MENU_SUMMARY:
            handleSummaryReport(&list);
            break;
        case MENU_EXPORT_REPORT:
            handleExportReport(&list);
            break;
        case MENU_EXIT:
            running = false;
            break;
        case MENU_INVALID:
            printf("Unsupported menu choice. Enter a number from 0 to 9.\n");
            break;
        }
    }

    Status_t status = saveObservations(&list, dataPath);

    if (status != STATUS_SUCCESS)
    {
        fprintf(stderr, "Error saving %s: %s\n", dataPath, getStatusMessage(status));
        return EXIT_FAILURE;
    }

    printf("Saved %d record(s) to %s. Clear skies!\n", list.count, dataPath);
    return EXIT_SUCCESS;
}
