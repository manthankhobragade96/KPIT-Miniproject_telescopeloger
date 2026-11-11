#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdbool.h>

#include "data_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Field validators: return true when the value is acceptable. */
bool isValidId(int id);
bool isValidTargetName(const char *name);
bool isValidObjectType(int typeValue);
bool isValidDate(Date_t date);
bool isValidExposure(int seconds);
bool isValidFrameCount(int frames);
bool isValidSeeing(int seeing);
bool isValidNotes(const char *notes);

/* Checks every field of a record; returns the status of the first failed field. */
Status_t validateObservation(const Observation_t *obs);

/* Parses text in DD-MM-YYYY format; *date is changed only on success. */
Status_t parseDate(const char *text, Date_t *date);

/* Strictly parses a whole decimal integer (surrounding whitespace allowed). */
Status_t parseInteger(const char *text, int *value);

#ifdef __cplusplus
}
#endif

#endif /* VALIDATION_H */
