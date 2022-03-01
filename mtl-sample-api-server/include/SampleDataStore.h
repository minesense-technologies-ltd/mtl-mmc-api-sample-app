#pragma once

/*--------------------------------------------------------------------------------------
* Copyright (c) 2005-2019 by MineSense Technologies Ltd.("Minesense"). All rights
* reserved.
*
* The copyright to the computer software herein is the property of MineSense.
* The software may be used and/or copied only with the written permission of
* MineSense or in accordance with the terms and conditions stipulated in
* the agreement/contract under which the software has been supplied.
*------------------------------------------------------------------------------------*/

#include "nlohmann/json.hpp"

#include <string>
#include <sys/time.h>

class SampleDataStore
{
public:
    SampleDataStore();
    ~SampleDataStore();

    /** GetGradeResponse
     * Returns an API GradeResponse object with test data corresponding to the input from a GradeRequest
     *
     * @param requestId - ID mapping a request to a response
     * @param haulingUnitId - Identifies the machine to retrieve grade data from
     * @param startTime - Beginning of the time range to request grade data from. UTC formatted timestring
     * @param endTime - End of the time range to request grade data from. UTC formatted timestring
     * @param grades - JSON-formatted collection of grade estimate data
     * @return JSON-formatted string containing a GradeResponse message
     */
    std::string GetGradeResponse(std::string requestId, std::string haulingUnitId, 
                                 std::string startTime, std::string endTime, nlohmann::json grades);

    /** RandomizeTimeValues_
     * Helper function to randomize the time values of a gradeResponse message
     *
     * @param responseJson - A json object containing a gradeResponse message
     * @param startTime - The gradeRequest's start time
     * @param endTime - The gradeRequest's end time
     */
    void RandomizeTimeValues_(nlohmann::json& responseJson, std::string startTime, std::string endTime);

    /** RandomizeGradeValue_
     * Helper function to randomize the grade value of a json object containing grade data
     *
     * @param gradeJson - A json object containing an entry with key "element"
     */
    void RandomizeGradeValue_(nlohmann::json& gradeJson);

    /** GetCurrentTimeAsString
     * Returns the current time in string form "[YYYY]-[MM]-[DD]T[HH]:[MM]:[SS].[XXX]Z", where the XXX is MS
     *
     * @return String containing the current time
     */
    std::string GetCurrentTimeAsString();

    /** TimevalToString
     * Returns the time in string form "[YYYY]-[MM]-[DD]T[HH]:[MM]:[SS].[XXX]Z", where the XXX is MS
     *
     * @param timeval
     * @return String containing the time
     */
    std::string TimevalToString(timeval tv);

    /** StringToTimeval
     * Converts a utc timestamp to a timeval. Requires utcTimestamp to be well formatted
     *
     * @param utcTimestamp
     * @return timeval
     */
    timeval StringToTimeval(std::string utcTimestamp);

private:
    std::string const mk_keyType            = "type";

    std::string const mk_responseType       = "gradeResponse";
    std::string const mk_keySystemId        = "systemID";
    std::string const mk_valueSystemId      = "testServer";
    std::string const mk_keyVersion         = "version";
    std::string const mk_valueVersion       = "2.3";

    std::string const mk_keyRequestId       = "requestID";
    std::string const mk_keyHaulingUnitId   = "haulingUnitID";
    std::string const mk_keyBatchStart      = "batchStart";
    std::string const mk_keyBatchEnd        = "batchEnd";
    std::string const mk_keyBatchId         = "batchID";
    std::string const mk_keyGrades          = "grades";
    std::string const mk_keyGradeValue      = "value";
    std::string const mk_keyGradeElement    = "element";
    std::string const mk_keyGradeConfidence = "confidenceScore";

    // The string buffer length to get the time
    unsigned const mk_stringBuffLength = 64;

    //                                                                                     Ms Index-v    v-Timezone Index
    // Indices used for parsing timestamps in the following format: [YYYY]-[MM]-[DD]T[HH]:[MM]:[SS].[XXX]Z
    unsigned const mk_timestampMsIndex = 20;
    unsigned const mk_timestampMsLength = 3;
    unsigned const mk_timestampTimezoneIndex = 23;
    unsigned const mk_timestampTimezoneMaxLength = 6; // Can be as short as 1 char for Z, or up to 6 with [-/+]HH:MM

    // Used for rounding random grade to decimal precision 2
    unsigned const mk_roundingMagnitude = 100;
    float    const mk_roundingThreshold = 0.5;

    // Maximum diff between start and end times for randomized timestamps
    unsigned const mk_maxRunLength = 60;
};
