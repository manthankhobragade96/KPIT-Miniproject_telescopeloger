#include <gtest/gtest.h>

#include "test_helpers.hpp"

extern "C" {
#include "analysis.h"
#include "observation.h"
}

TEST(IntegrationTime, MultipliesExposureByFrames)
{
    Observation_t obs = makeObservation(1, "M42", OBJ_NEBULA, 1, 1, 2026, 120, 30, 4);
    EXPECT_EQ(3600L, calculateIntegrationSeconds(&obs));
}

TEST(IntegrationTime, MaximumInputsDoNotOverflow)
{
    Observation_t obs = makeObservation(1, "M42", OBJ_NEBULA, 1, 1, 2026,
                                        MAX_EXPOSURE_SECONDS, MAX_FRAME_COUNT, 3);
    EXPECT_EQ(36000000L, calculateIntegrationSeconds(&obs));
}

TEST(IntegrationTime, NullRecordGivesZero)
{
    EXPECT_EQ(0L, calculateIntegrationSeconds(nullptr));
}

TEST(QualityRating, ExcellentAtExactThresholds)
{
    EXPECT_EQ(QUALITY_EXCELLENT, determineQuality(EXCELLENT_MIN_SEEING, EXCELLENT_MIN_SECONDS));
    EXPECT_EQ(QUALITY_EXCELLENT, determineQuality(MAX_SEEING, 100000L));
}

TEST(QualityRating, JustBelowExcellentIsGood)
{
    EXPECT_EQ(QUALITY_GOOD, determineQuality(EXCELLENT_MIN_SEEING, EXCELLENT_MIN_SECONDS - 1));
    EXPECT_EQ(QUALITY_GOOD, determineQuality(GOOD_MIN_SEEING, 100000L));
}

TEST(QualityRating, GoodBoundary)
{
    EXPECT_EQ(QUALITY_GOOD, determineQuality(GOOD_MIN_SEEING, GOOD_MIN_SECONDS));
    EXPECT_EQ(QUALITY_POOR, determineQuality(GOOD_MIN_SEEING, GOOD_MIN_SECONDS - 1));
}

TEST(QualityRating, PoorSeeingIsAlwaysPoor)
{
    EXPECT_EQ(QUALITY_POOR, determineQuality(GOOD_MIN_SEEING - 1, 1000000L));
    EXPECT_EQ(QUALITY_POOR, determineQuality(MIN_SEEING, 0L));
}

TEST(QualityNames, MapsEnumToText)
{
    EXPECT_STREQ("Excellent", qualityToString(QUALITY_EXCELLENT));
    EXPECT_STREQ("Good", qualityToString(QUALITY_GOOD));
    EXPECT_STREQ("Poor", qualityToString(QUALITY_POOR));
}

class AnalysisListTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        initObservationList(&list);
        Observation_t a = makeObservation(1, "M31", OBJ_GALAXY, 1, 10, 2026, 100, 36, 5);
        Observation_t b = makeObservation(2, "m31", OBJ_GALAXY, 2, 10, 2026, 100, 18, 3);
        Observation_t c = makeObservation(3, "Moon", OBJ_MOON, 3, 10, 2026, 1, 100, 2);
        ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &a));
        ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &b));
        ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &c));
    }

    ObservationList_t list;
};

TEST_F(AnalysisListTest, TargetTotalSumsMatchingRecordsIgnoringCase)
{
    EXPECT_EQ(5400L, calculateTargetIntegrationSeconds(&list, "M31"));
    EXPECT_EQ(100L, calculateTargetIntegrationSeconds(&list, "Moon"));
}

TEST_F(AnalysisListTest, TargetTotalIsZeroForUnknownOrNull)
{
    EXPECT_EQ(0L, calculateTargetIntegrationSeconds(&list, "Saturn"));
    EXPECT_EQ(0L, calculateTargetIntegrationSeconds(&list, nullptr));
    EXPECT_EQ(0L, calculateTargetIntegrationSeconds(nullptr, "M31"));
}

TEST_F(AnalysisListTest, StatisticsSummariseAllRecords)
{
    SessionStats_t stats;
    ASSERT_EQ(STATUS_SUCCESS, calculateStatistics(&list, &stats));
    EXPECT_EQ(3, stats.totalSessions);
    EXPECT_EQ(5500L, stats.totalIntegrationSeconds);
    EXPECT_NEAR(10.0 / 3.0, stats.averageSeeing, 1e-9);
    EXPECT_EQ(1, stats.qualityCounts[QUALITY_EXCELLENT]);
    EXPECT_EQ(1, stats.qualityCounts[QUALITY_GOOD]);
    EXPECT_EQ(1, stats.qualityCounts[QUALITY_POOR]);
    EXPECT_EQ(2, stats.typeCounts[OBJ_GALAXY]);
    EXPECT_EQ(1, stats.typeCounts[OBJ_MOON]);
    EXPECT_EQ(5400L, stats.bestTargetSeconds);
    EXPECT_STREQ("M31", stats.bestTarget);
}

TEST(Statistics, EmptyListIsReported)
{
    ObservationList_t empty;
    SessionStats_t stats;
    initObservationList(&empty);
    EXPECT_EQ(STATUS_EMPTY_COLLECTION, calculateStatistics(&empty, &stats));
}

TEST(Statistics, NullArgumentsAreRejected)
{
    ObservationList_t list;
    SessionStats_t stats;
    initObservationList(&list);
    EXPECT_EQ(STATUS_NULL_POINTER, calculateStatistics(nullptr, &stats));
    EXPECT_EQ(STATUS_NULL_POINTER, calculateStatistics(&list, nullptr));
}

TEST(SecondsToHours, ConvertsWholeAndFractionalHours)
{
    EXPECT_NEAR(1.0, secondsToHours(3600), 1e-9);
    EXPECT_NEAR(0.5, secondsToHours(1800), 1e-9);
    EXPECT_NEAR(0.0, secondsToHours(0), 1e-9);
}
