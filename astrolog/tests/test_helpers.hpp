#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include <cstdio>
#include <cstring>

extern "C" {
#include "data_model.h"
}

/* Builds a valid record; individual tests then change the field under test. */
inline Observation_t makeObservation(int id, const char *target, ObjectType_t type,
                                     int day, int month, int year,
                                     int exposure, int frames, int seeing)
{
    Observation_t obs;
    std::memset(&obs, 0, sizeof(obs));
    obs.id = id;
    std::snprintf(obs.target, sizeof(obs.target), "%s", target);
    obs.type = type;
    obs.date.day = day;
    obs.date.month = month;
    obs.date.year = year;
    obs.exposureSeconds = exposure;
    obs.frameCount = frames;
    obs.seeing = seeing;
    return obs;
}

inline Observation_t makeDefaultObservation(int id)
{
    return makeObservation(id, "M31", OBJ_GALAXY, 15, 8, 2026, 60, 10, 3);
}

#endif /* TEST_HELPERS_HPP */
