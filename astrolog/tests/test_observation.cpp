#include <gtest/gtest.h>

#include "test_helpers.hpp"

extern "C" {
#include "observation.h"
}

class ObservationListTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        initObservationList(&list);
    }

    void addSampleRecords()
    {
        Observation_t a = makeObservation(1, "Orion Nebula", OBJ_NEBULA, 10, 10, 2026, 60, 10, 4);
        Observation_t b = makeObservation(2, "Moon", OBJ_MOON, 5, 9, 2026, 1, 100, 3);
        Observation_t c = makeObservation(3, "Andromeda Galaxy", OBJ_GALAXY, 20, 11, 2026, 120, 40, 5);
        ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &a));
        ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &b));
        ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &c));
    }

    ObservationList_t list;
};

TEST_F(ObservationListTest, AddValidRecordIncreasesCount)
{
    Observation_t obs = makeDefaultObservation(1);
    EXPECT_EQ(STATUS_SUCCESS, addObservation(&list, &obs));
    EXPECT_EQ(1, list.count);
    EXPECT_EQ(1, list.items[0].id);
}

TEST_F(ObservationListTest, AddDuplicateIdIsRejected)
{
    Observation_t obs = makeDefaultObservation(7);
    ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &obs));
    EXPECT_EQ(STATUS_DUPLICATE_ID, addObservation(&list, &obs));
    EXPECT_EQ(1, list.count);
}

TEST_F(ObservationListTest, AddInvalidRecordIsRejectedWithSpecificStatus)
{
    Observation_t obs = makeDefaultObservation(1);
    obs.exposureSeconds = 0;
    EXPECT_EQ(STATUS_INVALID_EXPOSURE, addObservation(&list, &obs));
    EXPECT_EQ(0, list.count);
}

TEST_F(ObservationListTest, AddRejectsNullPointers)
{
    Observation_t obs = makeDefaultObservation(1);
    EXPECT_EQ(STATUS_NULL_POINTER, addObservation(nullptr, &obs));
    EXPECT_EQ(STATUS_NULL_POINTER, addObservation(&list, nullptr));
}

TEST_F(ObservationListTest, AddFailsWhenListIsFull)
{
    for (int id = 1; id <= MAX_OBSERVATIONS; id++)
    {
        Observation_t obs = makeDefaultObservation(id);
        ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &obs));
    }

    Observation_t extra = makeDefaultObservation(MAX_OBSERVATIONS + 1);
    EXPECT_EQ(STATUS_CAPACITY_FULL, addObservation(&list, &extra));
    EXPECT_EQ(MAX_OBSERVATIONS, list.count);
}

TEST_F(ObservationListTest, FindByIdReturnsIndex)
{
    addSampleRecords();
    int index = -1;
    EXPECT_EQ(STATUS_SUCCESS, findObservationIndexById(&list, 2, &index));
    EXPECT_EQ(1, index);
}

TEST_F(ObservationListTest, FindByIdReportsMissingRecord)
{
    addSampleRecords();
    int index = -1;
    EXPECT_EQ(STATUS_NOT_FOUND, findObservationIndexById(&list, 99, &index));
    EXPECT_EQ(-1, index);
}

TEST_F(ObservationListTest, FindByTargetIsCaseInsensitiveAndPartial)
{
    addSampleRecords();
    int indices[MAX_OBSERVATIONS];
    int found = findObservationsByTarget(&list, "NEBULA", indices, MAX_OBSERVATIONS);
    ASSERT_EQ(1, found);
    EXPECT_EQ(0, indices[0]);
}

TEST_F(ObservationListTest, FindByTargetWithNoMatchReturnsZero)
{
    addSampleRecords();
    int indices[MAX_OBSERVATIONS];
    EXPECT_EQ(0, findObservationsByTarget(&list, "Saturn", indices, MAX_OBSERVATIONS));
}

TEST_F(ObservationListTest, FindByTargetRejectsBadArguments)
{
    int indices[MAX_OBSERVATIONS];
    EXPECT_EQ(-1, findObservationsByTarget(&list, "", indices, MAX_OBSERVATIONS));
    EXPECT_EQ(-1, findObservationsByTarget(&list, nullptr, indices, MAX_OBSERVATIONS));
    EXPECT_EQ(-1, findObservationsByTarget(nullptr, "M31", indices, MAX_OBSERVATIONS));
}

TEST_F(ObservationListTest, UpdateReplacesDataAndKeepsId)
{
    addSampleRecords();
    Observation_t changed = makeObservation(999, "Jupiter", OBJ_PLANET, 1, 12, 2026, 2, 500, 2);
    ASSERT_EQ(STATUS_SUCCESS, updateObservation(&list, 2, &changed));
    EXPECT_EQ(2, list.items[1].id);
    EXPECT_STREQ("Jupiter", list.items[1].target);
    EXPECT_EQ(OBJ_PLANET, list.items[1].type);
}

TEST_F(ObservationListTest, UpdateUnknownIdIsRejected)
{
    addSampleRecords();
    Observation_t changed = makeDefaultObservation(1);
    EXPECT_EQ(STATUS_NOT_FOUND, updateObservation(&list, 50, &changed));
}

TEST_F(ObservationListTest, UpdateWithInvalidDataLeavesRecordUnchanged)
{
    addSampleRecords();
    Observation_t changed = makeDefaultObservation(1);
    changed.seeing = 0;
    EXPECT_EQ(STATUS_INVALID_SEEING, updateObservation(&list, 1, &changed));
    EXPECT_STREQ("Orion Nebula", list.items[0].target);
}

TEST_F(ObservationListTest, DeleteMiddleRecordClosesGap)
{
    addSampleRecords();
    ASSERT_EQ(STATUS_SUCCESS, deleteObservation(&list, 2));
    EXPECT_EQ(2, list.count);
    EXPECT_EQ(1, list.items[0].id);
    EXPECT_EQ(3, list.items[1].id);
}

TEST_F(ObservationListTest, DeleteMissingRecordIsReported)
{
    addSampleRecords();
    EXPECT_EQ(STATUS_NOT_FOUND, deleteObservation(&list, 42));
    EXPECT_EQ(3, list.count);
}

TEST_F(ObservationListTest, DeleteFromEmptyListIsReported)
{
    EXPECT_EQ(STATUS_EMPTY_COLLECTION, deleteObservation(&list, 1));
}

TEST_F(ObservationListTest, FilterByObjectTypeReturnsMatchingIndices)
{
    addSampleRecords();
    int indices[MAX_OBSERVATIONS];
    ASSERT_EQ(1, filterByObjectType(&list, OBJ_MOON, indices, MAX_OBSERVATIONS));
    EXPECT_EQ(1, indices[0]);
    EXPECT_EQ(0, filterByObjectType(&list, OBJ_STAR, indices, MAX_OBSERVATIONS));
    EXPECT_EQ(-1, filterByObjectType(&list, OBJ_TYPE_COUNT, indices, MAX_OBSERVATIONS));
}

TEST_F(ObservationListTest, FilterByDateRangeIncludesBothEndDates)
{
    addSampleRecords();
    int indices[MAX_OBSERVATIONS];
    Date_t from = {5, 9, 2026};
    Date_t to = {10, 10, 2026};
    EXPECT_EQ(2, filterByDateRange(&list, from, to, indices, MAX_OBSERVATIONS));
}

TEST_F(ObservationListTest, FilterByDateRangeRejectsReversedRange)
{
    addSampleRecords();
    int indices[MAX_OBSERVATIONS];
    Date_t from = {1, 12, 2026};
    Date_t to = {1, 1, 2026};
    EXPECT_EQ(-1, filterByDateRange(&list, from, to, indices, MAX_OBSERVATIONS));
}

TEST_F(ObservationListTest, SortByDateOrdersOldestFirst)
{
    addSampleRecords();
    ASSERT_EQ(STATUS_SUCCESS, sortObservations(&list, SORT_BY_DATE));
    EXPECT_EQ(2, list.items[0].id);
    EXPECT_EQ(1, list.items[1].id);
    EXPECT_EQ(3, list.items[2].id);
}

TEST_F(ObservationListTest, SortByIntegrationOrdersLongestFirst)
{
    addSampleRecords();
    ASSERT_EQ(STATUS_SUCCESS, sortObservations(&list, SORT_BY_INTEGRATION));
    EXPECT_EQ(3, list.items[0].id);
    EXPECT_EQ(1, list.items[1].id);
    EXPECT_EQ(2, list.items[2].id);
}

TEST_F(ObservationListTest, SortRejectsUnknownKeyAndHandlesEmptyList)
{
    EXPECT_EQ(STATUS_SUCCESS, sortObservations(&list, SORT_BY_DATE));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, sortObservations(&list, static_cast<SortKey_t>(99)));
    EXPECT_EQ(STATUS_NULL_POINTER, sortObservations(nullptr, SORT_BY_DATE));
}

TEST_F(ObservationListTest, NextFreeIdFillsGaps)
{
    EXPECT_EQ(1, getNextFreeId(&list));
    Observation_t one = makeDefaultObservation(1);
    Observation_t two = makeDefaultObservation(2);
    Observation_t four = makeDefaultObservation(4);
    ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &one));
    ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &two));
    ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &four));
    EXPECT_EQ(3, getNextFreeId(&list));
}

TEST(DateComparison, OrdersByYearThenMonthThenDay)
{
    Date_t early = {31, 12, 2025};
    Date_t late = {1, 1, 2026};
    Date_t same = {1, 1, 2026};
    EXPECT_EQ(-1, compareDates(early, late));
    EXPECT_EQ(1, compareDates(late, early));
    EXPECT_EQ(0, compareDates(late, same));
}

TEST(ObjectTypeNames, MapsEnumToText)
{
    EXPECT_STREQ("Nebula", objectTypeToString(OBJ_NEBULA));
    EXPECT_STREQ("Unknown", objectTypeToString(OBJ_TYPE_COUNT));
}
