#include <gtest/gtest.h>

#include <string>

#include "test_helpers.hpp"

extern "C" {
#include "validation.h"
}

TEST(DateValidation, AcceptsNormalDate)
{
    Date_t date = {15, 8, 2026};
    EXPECT_TRUE(isValidDate(date));
}

TEST(DateValidation, RejectsMonthThirteen)
{
    Date_t date = {10, 13, 2026};
    EXPECT_FALSE(isValidDate(date));
}

TEST(DateValidation, RejectsDayZero)
{
    Date_t date = {0, 5, 2026};
    EXPECT_FALSE(isValidDate(date));
}

TEST(DateValidation, RejectsThirtyFirstOfApril)
{
    Date_t date = {31, 4, 2026};
    EXPECT_FALSE(isValidDate(date));
}

TEST(DateValidation, LeapDayAcceptedOnlyInLeapYears)
{
    Date_t leap = {29, 2, 2028};
    Date_t common = {29, 2, 2027};
    EXPECT_TRUE(isValidDate(leap));
    EXPECT_FALSE(isValidDate(common));
}

TEST(DateValidation, YearBoundaries)
{
    Date_t first = {1, 1, MIN_YEAR};
    Date_t last = {31, 12, MAX_YEAR};
    Date_t tooEarly = {31, 12, MIN_YEAR - 1};
    Date_t tooLate = {1, 1, MAX_YEAR + 1};
    EXPECT_TRUE(isValidDate(first));
    EXPECT_TRUE(isValidDate(last));
    EXPECT_FALSE(isValidDate(tooEarly));
    EXPECT_FALSE(isValidDate(tooLate));
}

TEST(ExposureValidation, BoundaryValues)
{
    EXPECT_FALSE(isValidExposure(MIN_EXPOSURE_SECONDS - 1));
    EXPECT_TRUE(isValidExposure(MIN_EXPOSURE_SECONDS));
    EXPECT_TRUE(isValidExposure(MAX_EXPOSURE_SECONDS));
    EXPECT_FALSE(isValidExposure(MAX_EXPOSURE_SECONDS + 1));
}

TEST(FrameCountValidation, BoundaryValues)
{
    EXPECT_FALSE(isValidFrameCount(MIN_FRAME_COUNT - 1));
    EXPECT_TRUE(isValidFrameCount(MIN_FRAME_COUNT));
    EXPECT_TRUE(isValidFrameCount(MAX_FRAME_COUNT));
    EXPECT_FALSE(isValidFrameCount(MAX_FRAME_COUNT + 1));
}

TEST(SeeingValidation, BoundaryValues)
{
    EXPECT_FALSE(isValidSeeing(MIN_SEEING - 1));
    EXPECT_TRUE(isValidSeeing(MIN_SEEING));
    EXPECT_TRUE(isValidSeeing(MAX_SEEING));
    EXPECT_FALSE(isValidSeeing(MAX_SEEING + 1));
}

TEST(IdValidation, BoundaryValues)
{
    EXPECT_FALSE(isValidId(0));
    EXPECT_TRUE(isValidId(MIN_ID));
    EXPECT_TRUE(isValidId(MAX_ID));
    EXPECT_FALSE(isValidId(MAX_ID + 1));
}

TEST(TargetValidation, AcceptsTypicalName)
{
    EXPECT_TRUE(isValidTargetName("Orion Nebula M42"));
}

TEST(TargetValidation, RejectsNullEmptyAndBlank)
{
    EXPECT_FALSE(isValidTargetName(nullptr));
    EXPECT_FALSE(isValidTargetName(""));
    EXPECT_FALSE(isValidTargetName("    "));
}

TEST(TargetValidation, RejectsFileDelimiter)
{
    EXPECT_FALSE(isValidTargetName("M31|Andromeda"));
}

TEST(TargetValidation, LengthBoundary)
{
    EXPECT_TRUE(isValidTargetName(std::string(MAX_TARGET_LEN, 'a').c_str()));
    EXPECT_FALSE(isValidTargetName(std::string(MAX_TARGET_LEN + 1, 'a').c_str()));
}

TEST(NotesValidation, AllowsEmptyAndMaximumLength)
{
    EXPECT_TRUE(isValidNotes(""));
    EXPECT_TRUE(isValidNotes(std::string(MAX_NOTES_LEN, 'n').c_str()));
}

TEST(NotesValidation, RejectsTooLongNullAndDelimiter)
{
    EXPECT_FALSE(isValidNotes(std::string(MAX_NOTES_LEN + 1, 'n').c_str()));
    EXPECT_FALSE(isValidNotes(nullptr));
    EXPECT_FALSE(isValidNotes("bad|note"));
}

TEST(ObjectTypeValidation, BoundaryValues)
{
    EXPECT_FALSE(isValidObjectType(-1));
    EXPECT_TRUE(isValidObjectType(0));
    EXPECT_TRUE(isValidObjectType(OBJ_TYPE_COUNT - 1));
    EXPECT_FALSE(isValidObjectType(OBJ_TYPE_COUNT));
}

TEST(ValidateObservation, AcceptsValidRecord)
{
    Observation_t obs = makeDefaultObservation(1);
    EXPECT_EQ(STATUS_SUCCESS, validateObservation(&obs));
}

TEST(ValidateObservation, ReportsFirstInvalidField)
{
    Observation_t obs = makeDefaultObservation(1);
    obs.id = 0;
    EXPECT_EQ(STATUS_INVALID_ID, validateObservation(&obs));

    obs = makeDefaultObservation(1);
    obs.exposureSeconds = 0;
    EXPECT_EQ(STATUS_INVALID_EXPOSURE, validateObservation(&obs));

    obs = makeDefaultObservation(1);
    obs.frameCount = MAX_FRAME_COUNT + 1;
    EXPECT_EQ(STATUS_INVALID_FRAME_COUNT, validateObservation(&obs));

    obs = makeDefaultObservation(1);
    obs.seeing = 9;
    EXPECT_EQ(STATUS_INVALID_SEEING, validateObservation(&obs));
}

TEST(ValidateObservation, RejectsNullPointer)
{
    EXPECT_EQ(STATUS_NULL_POINTER, validateObservation(nullptr));
}

TEST(ParseDate, AcceptsDayMonthYear)
{
    Date_t date = {0, 0, 0};
    ASSERT_EQ(STATUS_SUCCESS, parseDate("15-08-2026", &date));
    EXPECT_EQ(15, date.day);
    EXPECT_EQ(8, date.month);
    EXPECT_EQ(2026, date.year);
}

TEST(ParseDate, RejectsWrongFormatAndTrailingText)
{
    Date_t date = {1, 1, 2001};
    EXPECT_EQ(STATUS_INVALID_DATE, parseDate("2026-08-15", &date));
    EXPECT_EQ(STATUS_INVALID_DATE, parseDate("15-08-2026x", &date));
    EXPECT_EQ(STATUS_INVALID_DATE, parseDate("31-02-2026", &date));
    EXPECT_EQ(STATUS_INVALID_DATE, parseDate("", &date));
    EXPECT_EQ(2001, date.year);
}

TEST(ParseDate, RejectsNullPointers)
{
    Date_t date = {1, 1, 2001};
    EXPECT_EQ(STATUS_NULL_POINTER, parseDate(nullptr, &date));
    EXPECT_EQ(STATUS_NULL_POINTER, parseDate("15-08-2026", nullptr));
}

TEST(ParseInteger, AcceptsPlainAndPaddedNumbers)
{
    int value = 0;
    ASSERT_EQ(STATUS_SUCCESS, parseInteger("42", &value));
    EXPECT_EQ(42, value);
    ASSERT_EQ(STATUS_SUCCESS, parseInteger("  -7  ", &value));
    EXPECT_EQ(-7, value);
}

TEST(ParseInteger, RejectsTextEmptyAndOverflow)
{
    int value = 5;
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, parseInteger("abc", &value));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, parseInteger("12abc", &value));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, parseInteger("", &value));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, parseInteger("99999999999999999999", &value));
    EXPECT_EQ(5, value);
}
