#ifndef OBSERVATION_H
#define OBSERVATION_H

#include "data_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Empties the list. */
void initObservationList(ObservationList_t *list);

/* Validates and appends a record; rejects duplicate IDs and a full list. */
Status_t addObservation(ObservationList_t *list, const Observation_t *obs);

/* Finds the array index of the record with the given ID. */
Status_t findObservationIndexById(const ObservationList_t *list, int id, int *indexOut);

/*
 * Searches, filters: write matching array indices to indices[] (at most maxIndices)
 * and return how many matched, or -1 when an argument is invalid.
 */
int findObservationsByTarget(const ObservationList_t *list, const char *keyword,
                             int *indices, int maxIndices);
int filterByObjectType(const ObservationList_t *list, ObjectType_t type,
                       int *indices, int maxIndices);
int filterByDateRange(const ObservationList_t *list, Date_t from, Date_t to,
                      int *indices, int maxIndices);

/* Replaces the data of record id (the ID itself never changes). */
Status_t updateObservation(ObservationList_t *list, int id, const Observation_t *newData);

/* Removes the record with the given ID and closes the gap. */
Status_t deleteObservation(ObservationList_t *list, int id);

/* Stable sort: SORT_BY_DATE ascending, SORT_BY_INTEGRATION longest first. */
Status_t sortObservations(ObservationList_t *list, SortKey_t key);

/* Smallest unused valid ID. */
int getNextFreeId(const ObservationList_t *list);

/* Returns -1, 0 or 1 when a is before, equal to or after b. */
int compareDates(Date_t a, Date_t b);

/* Display name of an object type. */
const char *objectTypeToString(ObjectType_t type);

#ifdef __cplusplus
}
#endif

#endif /* OBSERVATION_H */
