#include "observation.h"

#include <stdbool.h>
#include <string.h>

#include "analysis.h"
#include "text_utils.h"
#include "validation.h"

void initObservationList(ObservationList_t *list)
{
    if (list != NULL)
    {
        list->count = 0;
    }
}

Status_t findObservationIndexById(const ObservationList_t *list, int id, int *indexOut)
{
    if (list == NULL || indexOut == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    for (int i = 0; i < list->count; i++)
    {
        if (list->items[i].id == id)
        {
            *indexOut = i;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}

Status_t addObservation(ObservationList_t *list, const Observation_t *obs)
{
    if (list == NULL || obs == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    Status_t status = validateObservation(obs);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    int existingIndex = 0;

    if (findObservationIndexById(list, obs->id, &existingIndex) == STATUS_SUCCESS)
    {
        return STATUS_DUPLICATE_ID;
    }

    if (list->count >= MAX_OBSERVATIONS)
    {
        return STATUS_CAPACITY_FULL;
    }

    list->items[list->count] = *obs;
    list->count++;
    return STATUS_SUCCESS;
}

int findObservationsByTarget(const ObservationList_t *list, const char *keyword,
                             int *indices, int maxIndices)
{
    if (list == NULL || keyword == NULL || isBlankString(keyword) ||
        indices == NULL || maxIndices < 0)
    {
        return -1;
    }

    int found = 0;

    for (int i = 0; i < list->count && found < maxIndices; i++)
    {
        if (stringContainsIgnoreCase(list->items[i].target, keyword))
        {
            indices[found] = i;
            found++;
        }
    }

    return found;
}

int filterByObjectType(const ObservationList_t *list, ObjectType_t type,
                       int *indices, int maxIndices)
{
    if (list == NULL || indices == NULL || maxIndices < 0 || !isValidObjectType((int)type))
    {
        return -1;
    }

    int found = 0;

    for (int i = 0; i < list->count && found < maxIndices; i++)
    {
        if (list->items[i].type == type)
        {
            indices[found] = i;
            found++;
        }
    }

    return found;
}

int filterByDateRange(const ObservationList_t *list, Date_t from, Date_t to,
                      int *indices, int maxIndices)
{
    if (list == NULL || indices == NULL || maxIndices < 0 ||
        !isValidDate(from) || !isValidDate(to) || compareDates(from, to) > 0)
    {
        return -1;
    }

    int found = 0;

    for (int i = 0; i < list->count && found < maxIndices; i++)
    {
        const Date_t date = list->items[i].date;

        if (compareDates(date, from) >= 0 && compareDates(date, to) <= 0)
        {
            indices[found] = i;
            found++;
        }
    }

    return found;
}

Status_t updateObservation(ObservationList_t *list, int id, const Observation_t *newData)
{
    if (list == NULL || newData == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    Observation_t updated = *newData;
    updated.id = id;

    Status_t status = validateObservation(&updated);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    int index = 0;
    status = findObservationIndexById(list, id, &index);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    list->items[index] = updated;
    return STATUS_SUCCESS;
}

Status_t deleteObservation(ObservationList_t *list, int id)
{
    if (list == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (list->count == 0)
    {
        return STATUS_EMPTY_COLLECTION;
    }

    int index = 0;
    Status_t status = findObservationIndexById(list, id, &index);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    for (int i = index; i < list->count - 1; i++)
    {
        list->items[i] = list->items[i + 1];
    }

    list->count--;
    return STATUS_SUCCESS;
}

/* Strict ordering used by the insertion sort, which keeps equal records in order. */
static bool comesBefore(const Observation_t *first, const Observation_t *second, SortKey_t key)
{
    if (key == SORT_BY_DATE)
    {
        return compareDates(first->date, second->date) < 0;
    }

    return calculateIntegrationSeconds(first) > calculateIntegrationSeconds(second);
}

Status_t sortObservations(ObservationList_t *list, SortKey_t key)
{
    if (list == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (key != SORT_BY_DATE && key != SORT_BY_INTEGRATION)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    for (int i = 1; i < list->count; i++)
    {
        Observation_t current = list->items[i];
        int j = i - 1;

        while (j >= 0 && comesBefore(&current, &list->items[j], key))
        {
            list->items[j + 1] = list->items[j];
            j--;
        }

        list->items[j + 1] = current;
    }

    return STATUS_SUCCESS;
}

int getNextFreeId(const ObservationList_t *list)
{
    if (list == NULL)
    {
        return MIN_ID;
    }

    int candidate = MIN_ID;
    int index = 0;

    while (findObservationIndexById(list, candidate, &index) == STATUS_SUCCESS)
    {
        candidate++;
    }

    return candidate;
}

int compareDates(Date_t a, Date_t b)
{
    if (a.year != b.year)
    {
        return (a.year < b.year) ? -1 : 1;
    }

    if (a.month != b.month)
    {
        return (a.month < b.month) ? -1 : 1;
    }

    if (a.day != b.day)
    {
        return (a.day < b.day) ? -1 : 1;
    }

    return 0;
}

const char *objectTypeToString(ObjectType_t type)
{
    switch (type)
    {
    case OBJ_MOON:
        return "Moon";
    case OBJ_PLANET:
        return "Planet";
    case OBJ_STAR:
        return "Star";
    case OBJ_NEBULA:
        return "Nebula";
    case OBJ_GALAXY:
        return "Galaxy";
    case OBJ_CLUSTER:
        return "Cluster";
    case OBJ_OTHER:
        return "Other";
    case OBJ_TYPE_COUNT:
        break;
    }

    return "Unknown";
}
