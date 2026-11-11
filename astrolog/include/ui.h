#ifndef UI_H
#define UI_H

#include "data_model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MENU_INVALID = -1,
    MENU_EXIT = 0,
    MENU_ADD,
    MENU_LIST,
    MENU_SEARCH,
    MENU_EDIT,
    MENU_DELETE,
    MENU_FILTER,
    MENU_SORT,
    MENU_SUMMARY,
    MENU_EXPORT_REPORT
} MenuChoice_t;

/* Prints the main menu. */
void displayMenu(void);

/* Reads a menu choice; end of input is treated as MENU_EXIT. */
MenuChoice_t readMenuChoice(void);

/* One handler per menu option; each prompts, calls the other modules and reports. */
void handleAddObservation(ObservationList_t *list);
void handleListObservations(const ObservationList_t *list);
void handleSearchObservations(const ObservationList_t *list);
void handleEditObservation(ObservationList_t *list);
void handleDeleteObservation(ObservationList_t *list);
void handleFilterObservations(const ObservationList_t *list);
void handleSortObservations(ObservationList_t *list);
void handleSummaryReport(const ObservationList_t *list);
void handleExportReport(const ObservationList_t *list);

/* Human-readable message for a status code. */
const char *getStatusMessage(Status_t status);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
