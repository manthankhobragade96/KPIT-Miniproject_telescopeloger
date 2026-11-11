#include <gtest/gtest.h>

#include <cstdio>
#include <string>

#include "test_helpers.hpp"

extern "C" {
#include "observation.h"
#include "report.h"
}

TEST(FormatDuration, FormatsHoursMinutesSeconds)
{
    char buffer[32];
    formatDuration(3725, buffer, sizeof(buffer));
    EXPECT_STREQ("1h 02m 05s", buffer);
}

TEST(FormatDuration, ZeroAndNegativeShowZero)
{
    char buffer[32];
    formatDuration(0, buffer, sizeof(buffer));
    EXPECT_STREQ("0h 00m 00s", buffer);
    formatDuration(-50, buffer, sizeof(buffer));
    EXPECT_STREQ("0h 00m 00s", buffer);
}

TEST(FormatDuration, LargeValuesKeepAllHours)
{
    char buffer[32];
    formatDuration(36000000L, buffer, sizeof(buffer));
    EXPECT_STREQ("10000h 00m 00s", buffer);
}

TEST(FormatDuration, NullOrZeroSizedBufferIsIgnored)
{
    char buffer[4] = "abc";
    formatDuration(10, nullptr, 10);
    formatDuration(10, buffer, 0);
    EXPECT_STREQ("abc", buffer);
}

TEST(SummaryReport, EmptyListReportsNothingToSummarise)
{
    ObservationList_t list;
    initObservationList(&list);
    FILE *out = std::tmpfile();
    ASSERT_NE(nullptr, out);
    EXPECT_EQ(STATUS_EMPTY_COLLECTION, printSummaryReport(out, &list));
    std::fclose(out);
}

TEST(SummaryReport, ContainsTotalsAndBestTarget)
{
    ObservationList_t list;
    initObservationList(&list);
    Observation_t obs = makeObservation(1, "Orion Nebula", OBJ_NEBULA, 9, 10, 2026, 60, 60, 5);
    ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &obs));

    FILE *out = std::tmpfile();
    ASSERT_NE(nullptr, out);
    ASSERT_EQ(STATUS_SUCCESS, printSummaryReport(out, &list));
    std::rewind(out);

    std::string text;
    char line[256];
    while (std::fgets(line, sizeof(line), out) != nullptr)
    {
        text += line;
    }
    std::fclose(out);

    EXPECT_NE(std::string::npos, text.find("Total sessions     : 1"));
    EXPECT_NE(std::string::npos, text.find("1h 00m 00s"));
    EXPECT_NE(std::string::npos, text.find("Orion Nebula"));
}

TEST(SummaryReport, ExportToUnwritablePathFails)
{
    ObservationList_t list;
    initObservationList(&list);
    EXPECT_EQ(STATUS_FILE_OPEN_FAILED, exportSummaryReport(&list, "/nonexistent_dir_xyz/report.txt"));
}
