#include "report.h"

#include <string.h>

#include "analysis.h"
#include "observation.h"

#define SECONDS_PER_MINUTE 60L
#define SECONDS_PER_HOUR_LONG 3600L
#define DURATION_TEXT_SIZE 32
#define TABLE_TARGET_WIDTH 20
#define EXPOSURE_TEXT_SIZE 24
#define DATE_TEXT_SIZE 16

void formatDuration(long totalSeconds, char *buffer, size_t size)
{
    if (buffer == NULL || size == 0)
    {
        return;
    }

    if (totalSeconds < 0)
    {
        totalSeconds = 0;
    }

    long hours = totalSeconds / SECONDS_PER_HOUR_LONG;
    long minutes = (totalSeconds % SECONDS_PER_HOUR_LONG) / SECONDS_PER_MINUTE;
    long seconds = totalSeconds % SECONDS_PER_MINUTE;

    snprintf(buffer, size, "%ldh %02ldm %02lds", hours, minutes, seconds);
}

void printObservationTable(FILE *out, const ObservationList_t *list,
                           const int *indices, int count)
{
    if (out == NULL || list == NULL)
    {
        return;
    }

    if (count <= 0)
    {
        fprintf(out, "No records to display.\n");
        return;
    }

    fprintf(out, "%-6s %-10s %-*s %-8s %-12s %-13s %-3s %s\n", "ID", "Date",
            TABLE_TARGET_WIDTH, "Target", "Type", "Exp x Frames", "Total", "See",
            "Quality");

    for (int row = 0; row < count; row++)
    {
        int index = (indices != NULL) ? indices[row] : row;

        if (index < 0 || index >= list->count)
        {
            continue;
        }

        const Observation_t *obs = &list->items[index];
        long seconds = calculateIntegrationSeconds(obs);
        char dateText[DATE_TEXT_SIZE];
        char exposureText[EXPOSURE_TEXT_SIZE];
        char totalText[DURATION_TEXT_SIZE];

        snprintf(dateText, sizeof(dateText), "%02d-%02d-%04d",
                 obs->date.day, obs->date.month, obs->date.year);
        snprintf(exposureText, sizeof(exposureText), "%ds x %d",
                 obs->exposureSeconds, obs->frameCount);
        formatDuration(seconds, totalText, sizeof(totalText));

        fprintf(out, "%-6d %-10s %-*.*s %-8s %-12s %-13s %-3d %s\n", obs->id,
                dateText, TABLE_TARGET_WIDTH, TABLE_TARGET_WIDTH, obs->target,
                objectTypeToString(obs->type), exposureText, totalText, obs->seeing,
                qualityToString(determineQuality(obs->seeing, seconds)));
    }
}

Status_t printSummaryReport(FILE *out, const ObservationList_t *list)
{
    if (out == NULL || list == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    SessionStats_t stats;
    Status_t status = calculateStatistics(list, &stats);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    char totalText[DURATION_TEXT_SIZE];
    char bestText[DURATION_TEXT_SIZE];

    formatDuration(stats.totalIntegrationSeconds, totalText, sizeof(totalText));
    formatDuration(stats.bestTargetSeconds, bestText, sizeof(bestText));

    fprintf(out, "=========== ASTROLOG SUMMARY REPORT ===========\n");
    fprintf(out, "Total sessions     : %d\n", stats.totalSessions);
    fprintf(out, "Total integration  : %s (%.2f h)\n", totalText,
            secondsToHours(stats.totalIntegrationSeconds));
    fprintf(out, "Average seeing     : %.2f / %d\n", stats.averageSeeing, MAX_SEEING);
    fprintf(out, "Most imaged target : %s (%s)\n", stats.bestTarget, bestText);
    fprintf(out, "Quality ratings    : Excellent %d | Good %d | Poor %d\n",
            stats.qualityCounts[QUALITY_EXCELLENT], stats.qualityCounts[QUALITY_GOOD],
            stats.qualityCounts[QUALITY_POOR]);
    fprintf(out, "Sessions by object type:\n");

    for (int type = 0; type < OBJ_TYPE_COUNT; type++)
    {
        fprintf(out, "  %-8s : %d\n", objectTypeToString((ObjectType_t)type),
                stats.typeCounts[type]);
    }

    fprintf(out, "===============================================\n");

    return ferror(out) ? STATUS_FILE_WRITE_FAILED : STATUS_SUCCESS;
}

Status_t exportSummaryReport(const ObservationList_t *list, const char *path)
{
    if (list == NULL || path == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    FILE *file = fopen(path, "w");

    if (file == NULL)
    {
        return STATUS_FILE_OPEN_FAILED;
    }

    Status_t status = printSummaryReport(file, list);

    if (fclose(file) != 0 && status == STATUS_SUCCESS)
    {
        status = STATUS_FILE_WRITE_FAILED;
    }

    return status;
}
