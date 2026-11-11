#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "data_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Writes all records to path (via path.tmp, then rename, so a failed save never
 * destroys the previous file).
 */
Status_t saveObservations(const ObservationList_t *list, const char *path);

/*
 * Loads records from path into list. The list is emptied first and is left empty on
 * any failure. errorLine (optional, may be NULL) receives the 1-based line number
 * of the first bad line, or 0.
 */
Status_t loadObservations(ObservationList_t *list, const char *path, int *errorLine);

/* Converts one data-file line into a validated record. */
Status_t parseRecordLine(const char *line, Observation_t *obs);

#ifdef __cplusplus
}
#endif

#endif /* FILE_HANDLER_H */
