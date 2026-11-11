#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include "test_helpers.hpp"

extern "C" {
#include "file_handler.h"
#include "observation.h"
}

/* Each test gets its own temporary file, removed again in TearDown. */
class FileHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        char pattern[] = "/tmp/astrolog_test_XXXXXX";
        int descriptor = mkstemp(pattern);
        ASSERT_NE(-1, descriptor);
        close(descriptor);
        path = pattern;
        initObservationList(&list);
    }

    void TearDown() override
    {
        std::remove(path.c_str());
        std::remove((path + ".tmp").c_str());
    }

    void writeFile(const std::string &content)
    {
        FILE *file = std::fopen(path.c_str(), "w");
        ASSERT_NE(nullptr, file);
        std::fputs(content.c_str(), file);
        std::fclose(file);
    }

    std::string path;
    ObservationList_t list;
};

TEST_F(FileHandlerTest, SaveThenLoadRestoresAllFields)
{
    Observation_t first = makeObservation(1, "Orion Nebula", OBJ_NEBULA, 9, 10, 2026, 60, 90, 5);
    std::snprintf(first.notes, sizeof(first.notes), "%s", "Excellent night");
    Observation_t second = makeObservation(2, "Moon", OBJ_MOON, 5, 9, 2026, 1, 500, 3);
    ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &first));
    ASSERT_EQ(STATUS_SUCCESS, addObservation(&list, &second));
    ASSERT_EQ(STATUS_SUCCESS, saveObservations(&list, path.c_str()));

    ObservationList_t loaded;
    int errorLine = -1;
    ASSERT_EQ(STATUS_SUCCESS, loadObservations(&loaded, path.c_str(), &errorLine));
    EXPECT_EQ(0, errorLine);
    ASSERT_EQ(2, loaded.count);
    EXPECT_EQ(1, loaded.items[0].id);
    EXPECT_STREQ("Orion Nebula", loaded.items[0].target);
    EXPECT_EQ(OBJ_NEBULA, loaded.items[0].type);
    EXPECT_EQ(9, loaded.items[0].date.day);
    EXPECT_EQ(60, loaded.items[0].exposureSeconds);
    EXPECT_EQ(90, loaded.items[0].frameCount);
    EXPECT_EQ(5, loaded.items[0].seeing);
    EXPECT_STREQ("Excellent night", loaded.items[0].notes);
    EXPECT_EQ(2, loaded.items[1].id);
}

TEST_F(FileHandlerTest, SaveEmptyListThenLoadGivesEmptyList)
{
    ASSERT_EQ(STATUS_SUCCESS, saveObservations(&list, path.c_str()));
    ObservationList_t loaded;
    EXPECT_EQ(STATUS_SUCCESS, loadObservations(&loaded, path.c_str(), nullptr));
    EXPECT_EQ(0, loaded.count);
}

TEST_F(FileHandlerTest, SaveLeavesNoTemporaryFileBehind)
{
    ASSERT_EQ(STATUS_SUCCESS, saveObservations(&list, path.c_str()));
    EXPECT_NE(0, access((path + ".tmp").c_str(), F_OK));
}

TEST_F(FileHandlerTest, SaveToMissingDirectoryFailsToOpen)
{
    EXPECT_EQ(STATUS_FILE_OPEN_FAILED, saveObservations(&list, "/nonexistent_dir_xyz/data.txt"));
}

TEST_F(FileHandlerTest, LoadMissingFileIsReported)
{
    EXPECT_EQ(STATUS_FILE_NOT_FOUND, loadObservations(&list, "/tmp/astrolog_no_such_file.txt", nullptr));
}

TEST_F(FileHandlerTest, LoadEmptyFileIsReported)
{
    writeFile("");
    EXPECT_EQ(STATUS_FILE_EMPTY, loadObservations(&list, path.c_str(), nullptr));
}

TEST_F(FileHandlerTest, LoadHeaderOnlyFileGivesNoRecords)
{
    writeFile("ASTROLOG_V1\n");
    EXPECT_EQ(STATUS_SUCCESS, loadObservations(&list, path.c_str(), nullptr));
    EXPECT_EQ(0, list.count);
}

TEST_F(FileHandlerTest, LoadWrongHeaderIsInvalidDataOnLineOne)
{
    writeFile("SOMETHING_ELSE\n1|M31|4|15|8|2026|60|10|3|ok\n");
    int errorLine = 0;
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadObservations(&list, path.c_str(), &errorLine));
    EXPECT_EQ(1, errorLine);
}

TEST_F(FileHandlerTest, LoadMalformedRecordReportsLineAndEmptiesList)
{
    writeFile("ASTROLOG_V1\n1|M31|4|15|8|2026|60|10|3|ok\n2|Moon|0|5|9\n");
    int errorLine = 0;
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadObservations(&list, path.c_str(), &errorLine));
    EXPECT_EQ(3, errorLine);
    EXPECT_EQ(0, list.count);
}

TEST_F(FileHandlerTest, LoadDuplicateIdsIsInvalidData)
{
    writeFile("ASTROLOG_V1\n1|M31|4|15|8|2026|60|10|3|a\n1|Moon|0|5|9|2026|1|50|3|b\n");
    int errorLine = 0;
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, loadObservations(&list, path.c_str(), &errorLine));
    EXPECT_EQ(3, errorLine);
}

TEST_F(FileHandlerTest, LoadSkipsBlankLines)
{
    writeFile("ASTROLOG_V1\n\n1|M31|4|15|8|2026|60|10|3|a\n\n");
    EXPECT_EQ(STATUS_SUCCESS, loadObservations(&list, path.c_str(), nullptr));
    EXPECT_EQ(1, list.count);
}

TEST_F(FileHandlerTest, ParseRecordLineAcceptsValidLine)
{
    Observation_t obs;
    ASSERT_EQ(STATUS_SUCCESS, parseRecordLine("5|Saturn|1|30|10|2026|1|700|4|Rings\n", &obs));
    EXPECT_EQ(5, obs.id);
    EXPECT_STREQ("Saturn", obs.target);
    EXPECT_EQ(OBJ_PLANET, obs.type);
    EXPECT_EQ(2026, obs.date.year);
    EXPECT_STREQ("Rings", obs.notes);
}

TEST_F(FileHandlerTest, ParseRecordLineAllowsEmptyNotes)
{
    Observation_t obs;
    ASSERT_EQ(STATUS_SUCCESS, parseRecordLine("5|Saturn|1|30|10|2026|1|700|4|", &obs));
    EXPECT_STREQ("", obs.notes);
}

TEST_F(FileHandlerTest, ParseRecordLineRejectsBadLines)
{
    Observation_t obs;
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, parseRecordLine("5|Saturn|99|30|10|2026|1|700|4|x", &obs));
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, parseRecordLine("5|Saturn|1|31|2|2026|1|700|4|x", &obs));
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, parseRecordLine("5|Saturn|1|30|10|2026|1|700|9|x", &obs));
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, parseRecordLine("5|Saturn|1|30|10|2026|1|700|4|x|extra", &obs));
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, parseRecordLine("five|Saturn|1|30|10|2026|1|700|4|x", &obs));
    EXPECT_EQ(STATUS_FILE_INVALID_DATA, parseRecordLine("", &obs));
}

TEST_F(FileHandlerTest, NullArgumentsAreRejected)
{
    Observation_t obs;
    EXPECT_EQ(STATUS_NULL_POINTER, parseRecordLine(nullptr, &obs));
    EXPECT_EQ(STATUS_NULL_POINTER, parseRecordLine("x", nullptr));
    EXPECT_EQ(STATUS_NULL_POINTER, saveObservations(nullptr, path.c_str()));
    EXPECT_EQ(STATUS_NULL_POINTER, loadObservations(&list, nullptr, nullptr));
}
