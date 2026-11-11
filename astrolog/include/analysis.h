#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "data_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Total exposure time of one record in seconds (exposure x frames); 0 if obs is NULL. */
long calculateIntegrationSeconds(const Observation_t *obs);

/* Classifies an observation from its seeing rating and total integration time. */
Quality_t determineQuality(int seeing, long integrationSeconds);

/* Sum of integration time over all records whose target equals target (ignoring case). */
long calculateTargetIntegrationSeconds(const ObservationList_t *list, const char *target);

/* Fills stats from the list; returns STATUS_EMPTY_COLLECTION when there are no records. */
Status_t calculateStatistics(const ObservationList_t *list, SessionStats_t *stats);

/* Converts seconds to hours. */
double secondsToHours(long seconds);

/* Display name of a quality rating. */
const char *qualityToString(Quality_t quality);

#ifdef __cplusplus
}
#endif

#endif /* ANALYSIS_H */
