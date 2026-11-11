#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>
#include <stdio.h>

#include "data_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Writes "Xh YYm ZZs" into buffer (negative input is shown as zero). */
void formatDuration(long totalSeconds, char *buffer, size_t size);

/*
 * Prints a table of records. When indices is NULL the first count records are shown,
 * otherwise the records at indices[0..count-1].
 */
void printObservationTable(FILE *out, const ObservationList_t *list,
                           const int *indices, int count);

/* Prints the summary report; STATUS_EMPTY_COLLECTION when there is nothing to report. */
Status_t printSummaryReport(FILE *out, const ObservationList_t *list);

/* Writes the summary report to a text file. */
Status_t exportSummaryReport(const ObservationList_t *list, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* REPORT_H */
