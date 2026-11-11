#ifndef DATA_MODEL_H
#define DATA_MODEL_H

/*
 * data_model.h
 * Shared constants, enumerations and structures used by every module.
 * This header declares data only; it contains no function definitions.
 */

/* ---- Capacity and size limits ---- */
#define MAX_OBSERVATIONS 200
#define MAX_TARGET_LEN 40
#define MAX_NOTES_LEN 80
#define INPUT_BUFFER_SIZE 128
#define LINE_BUFFER_SIZE 256
#define TEMP_PATH_SIZE 260

/* ---- Valid ranges for record fields ---- */
#define MIN_ID 1
#define MAX_ID 999999
#define MIN_YEAR 2000
#define MAX_YEAR 2100
#define MIN_EXPOSURE_SECONDS 1
#define MAX_EXPOSURE_SECONDS 3600
#define MIN_FRAME_COUNT 1
#define MAX_FRAME_COUNT 10000
#define MIN_SEEING 1
#define MAX_SEEING 5

/* ---- Quality classification thresholds (see detailed design, section 6) ---- */
#define EXCELLENT_MIN_SEEING 4
#define EXCELLENT_MIN_SECONDS 3600L
#define GOOD_MIN_SEEING 3
#define GOOD_MIN_SECONDS 600L

/* ---- File format ---- */
#define FIELD_DELIMITER '|'
#define RECORD_FIELD_COUNT 10
#define DATA_FILE_HEADER "ASTROLOG_V1"
#define DEFAULT_DATA_FILE "data/observations.txt"
#define REPORT_FILE "data/report.txt"

/* ---- Status codes returned by most functions ---- */
typedef enum
{
    STATUS_SUCCESS = 0,
    STATUS_NULL_POINTER,
    STATUS_INVALID_ARGUMENT,
    STATUS_INVALID_ID,
    STATUS_INVALID_TARGET,
    STATUS_INVALID_TYPE,
    STATUS_INVALID_DATE,
    STATUS_INVALID_EXPOSURE,
    STATUS_INVALID_FRAME_COUNT,
    STATUS_INVALID_SEEING,
    STATUS_INVALID_NOTES,
    STATUS_DUPLICATE_ID,
    STATUS_NOT_FOUND,
    STATUS_EMPTY_COLLECTION,
    STATUS_CAPACITY_FULL,
    STATUS_FILE_NOT_FOUND,
    STATUS_FILE_OPEN_FAILED,
    STATUS_FILE_EMPTY,
    STATUS_FILE_INVALID_DATA,
    STATUS_FILE_READ_FAILED,
    STATUS_FILE_WRITE_FAILED,
    STATUS_INPUT_EOF,
    STATUS_INPUT_TOO_LONG,
    STATUS_CANCELLED
} Status_t;

/* ---- Category of the observed object ---- */
typedef enum
{
    OBJ_MOON = 0,
    OBJ_PLANET,
    OBJ_STAR,
    OBJ_NEBULA,
    OBJ_GALAXY,
    OBJ_CLUSTER,
    OBJ_OTHER,
    OBJ_TYPE_COUNT
} ObjectType_t;

/* ---- Derived quality rating of an observation ---- */
typedef enum
{
    QUALITY_POOR = 0,
    QUALITY_GOOD,
    QUALITY_EXCELLENT,
    QUALITY_COUNT
} Quality_t;

/* ---- Sort criteria ---- */
typedef enum
{
    SORT_BY_DATE = 0,
    SORT_BY_INTEGRATION
} SortKey_t;

/* ---- Calendar date (DD-MM-YYYY when typed or displayed) ---- */
typedef struct
{
    int day;
    int month;
    int year;
} Date_t;

/* ---- One observing session record ---- */
typedef struct
{
    int id;
    char target[MAX_TARGET_LEN + 1];
    ObjectType_t type;
    Date_t date;
    int exposureSeconds;
    int frameCount;
    int seeing;
    char notes[MAX_NOTES_LEN + 1];
} Observation_t;

/* ---- Fixed-capacity collection of records ---- */
typedef struct
{
    Observation_t items[MAX_OBSERVATIONS];
    int count;
} ObservationList_t;

/* ---- Aggregate figures for the summary report ---- */
typedef struct
{
    int totalSessions;
    long totalIntegrationSeconds;
    double averageSeeing;
    int qualityCounts[QUALITY_COUNT];
    int typeCounts[OBJ_TYPE_COUNT];
    char bestTarget[MAX_TARGET_LEN + 1];
    long bestTargetSeconds;
} SessionStats_t;

#endif /* DATA_MODEL_H */
