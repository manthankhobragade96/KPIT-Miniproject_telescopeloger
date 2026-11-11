#include "analysis.h"

#include <stdio.h>
#include <string.h>

#include "text_utils.h"

#define SECONDS_PER_HOUR 3600.0

long calculateIntegrationSeconds(const Observation_t *obs)
{
    if (obs == NULL)
    {
        return 0;
    }

    return (long)obs->exposureSeconds * (long)obs->frameCount;
}

Quality_t determineQuality(int seeing, long integrationSeconds)
{
    if (seeing >= EXCELLENT_MIN_SEEING && integrationSeconds >= EXCELLENT_MIN_SECONDS)
    {
        return QUALITY_EXCELLENT;
    }

    if (seeing >= GOOD_MIN_SEEING && integrationSeconds >= GOOD_MIN_SECONDS)
    {
        return QUALITY_GOOD;
    }

    return QUALITY_POOR;
}

long calculateTargetIntegrationSeconds(const ObservationList_t *list, const char *target)
{
    if (list == NULL || target == NULL)
    {
        return 0;
    }

    long total = 0;

    for (int i = 0; i < list->count; i++)
    {
        if (stringEqualsIgnoreCase(list->items[i].target, target))
        {
            total += calculateIntegrationSeconds(&list->items[i]);
        }
    }

    return total;
}

Status_t calculateStatistics(const ObservationList_t *list, SessionStats_t *stats)
{
    if (list == NULL || stats == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (list->count == 0)
    {
        return STATUS_EMPTY_COLLECTION;
    }

    memset(stats, 0, sizeof(*stats));
    stats->totalSessions = list->count;

    long seeingSum = 0;

    for (int i = 0; i < list->count; i++)
    {
        const Observation_t *obs = &list->items[i];
        long seconds = calculateIntegrationSeconds(obs);

        stats->totalIntegrationSeconds += seconds;
        seeingSum += obs->seeing;
        stats->qualityCounts[determineQuality(obs->seeing, seconds)]++;

        if (obs->type >= 0 && obs->type < OBJ_TYPE_COUNT)
        {
            stats->typeCounts[obs->type]++;
        }

        long targetSeconds = calculateTargetIntegrationSeconds(list, obs->target);

        if (targetSeconds > stats->bestTargetSeconds)
        {
            stats->bestTargetSeconds = targetSeconds;
            snprintf(stats->bestTarget, sizeof(stats->bestTarget), "%s", obs->target);
        }
    }

    stats->averageSeeing = (double)seeingSum / (double)list->count;
    return STATUS_SUCCESS;
}

double secondsToHours(long seconds)
{
    return (double)seconds / SECONDS_PER_HOUR;
}

const char *qualityToString(Quality_t quality)
{
    switch (quality)
    {
    case QUALITY_POOR:
        return "Poor";
    case QUALITY_GOOD:
        return "Good";
    case QUALITY_EXCELLENT:
        return "Excellent";
    case QUALITY_COUNT:
        break;
    }

    return "Unknown";
}
