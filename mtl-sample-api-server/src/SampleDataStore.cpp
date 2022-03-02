#include "SampleDataStore.h"

#include <iostream>
#include <cstdlib>

SampleDataStore::SampleDataStore()
{

}

SampleDataStore::~SampleDataStore()
{
    
}

std::string SampleDataStore::GetGradeResponse(std::string requestId, std::string haulingUnitId, 
                                              std::string startTime, std::string endTime, nlohmann::json grades)
{
    nlohmann::json response;

    response[mk_keyType] = mk_responseType;         // "type" is always "gradeResponse"
    response[mk_keyVersion] = mk_valueVersion;      // "version" is always "2.3"
    response[mk_keySystemId] = mk_valueSystemId;    // "systemId" is always "testServer"
    response[mk_keyRequestId] = requestId;          // "requestId" is the same as the given requestId
    response[mk_keyHaulingUnitId] = haulingUnitId;  // "haulingUnitId" is the same as the given haulingUnitId

    // "startTime" and "endTime" are randomly selected between the start and end times of the request
    RandomizeTimeValues_(response, startTime, endTime);

    // "batchID" is a random number between 1 and 100
    unsigned randBatchID = ((float)std::rand() / (float)RAND_MAX) * 100;
    response[mk_keyBatchId] = std::to_string(randBatchID);

    // "grades" is randomly generated using the timestamp average as a seed
    nlohmann::json newGrades;
    if (!grades.empty())
    {
        for (auto& gradeSet : grades)
        {
            RandomizeGradeValue_(gradeSet);
            newGrades.push_back(gradeSet);
        }
    }
    else
    {
        nlohmann::json defaultCopperGrade;
        defaultCopperGrade[mk_keyGradeElement] = "Cu";
        RandomizeGradeValue_(defaultCopperGrade);
        newGrades.push_back(defaultCopperGrade);
    }

    response[mk_keyGrades] = newGrades;

    return response.dump();
}

void SampleDataStore::RandomizeTimeValues_(nlohmann::json& responseJson, std::string startTime, std::string endTime)
{
    timeval startTimeTv   = StringToTimeval(startTime);
    timeval endTimeTv     = StringToTimeval(endTime);

    unsigned avgSeconds = (startTimeTv.tv_sec + endTimeTv.tv_sec)/2;
    std::srand(avgSeconds); // Use average timestamp as the random seed to provide consistency in repeated query results

    // Pick a random end time from the second half of the given timerange...
    unsigned lateRange =  endTimeTv.tv_sec - avgSeconds;
    float randSeconds = ((float)std::rand() / (float)RAND_MAX) * lateRange;

    timeval newEndTv;
    newEndTv.tv_sec = endTimeTv.tv_sec - randSeconds;
    newEndTv.tv_usec = 0;
    std::string endTimeString = TimevalToString(newEndTv);
    responseJson[mk_keyBatchEnd] = endTimeString;

    // ...And determine the start time by subtracting a number of seconds from the end time (capped at mk_maxRunLength)
    unsigned startEndInterval = lateRange;
    if (startEndInterval > mk_maxRunLength)
    {
        startEndInterval = mk_maxRunLength;
    }

    timeval newStartTv;
    newStartTv.tv_sec = newEndTv.tv_sec - startEndInterval;
    newStartTv.tv_usec = 0;
    std::string startTimeString = TimevalToString(newStartTv);
    responseJson[mk_keyBatchStart] = startTimeString;
}

void SampleDataStore::RandomizeGradeValue_(nlohmann::json& gradeJson)
{
    // Generate random float between 0 and 1, then round to nearest 2 decimal places
    float randFloat = (float)std::rand() / (float)RAND_MAX;
    randFloat = (int)(randFloat * mk_roundingMagnitude + mk_roundingThreshold);
    randFloat = (float) randFloat / mk_roundingMagnitude;

    gradeJson[mk_keyGradeValue] = randFloat;
    gradeJson[mk_keyGradeConfidence] = 1.0;
}

std::string SampleDataStore::GetCurrentTimeAsString()
{
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[mk_stringBuffLength], znbuf[mk_stringBuffLength], buf[mk_stringBuffLength];

    // get tm_gmtoff to account for timezone
    time(&nowtime);
    nowtm = localtime(&nowtime);

    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%dT%H:%M:%S", nowtm);

    strftime(znbuf, sizeof znbuf, "%z", nowtm);
    snprintf(buf, sizeof buf, "%s.%03ld%s", tmbuf, 0, znbuf);

    return buf;
}

std::string SampleDataStore::TimevalToString(timeval tv)
{
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[mk_stringBuffLength], znbuf[mk_stringBuffLength], buf[mk_stringBuffLength];

    // get tm_gmtoff to account for timezone
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);

    // recalculate localtime with timezone offset applied
    nowtime = tv.tv_sec + nowtm->tm_gmtoff;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%dT%H:%M:%S", nowtm);

    strftime(znbuf, sizeof znbuf, "%z", nowtm);
    snprintf(buf, sizeof buf, "%s.%03ld%s", tmbuf, tv.tv_usec, znbuf);

    return buf;
}

timeval SampleDataStore::StringToTimeval(std::string utcTimestamp)
{
    timeval tv;
    int k_microToUs = 1000;

    // Convert everything from the timestamp but milliseconds to seconds
    char buffer[mk_stringBuffLength];
    memset(buffer, '\0', sizeof(buffer));
    strncpy(buffer,
            utcTimestamp.c_str(),
            mk_timestampMsIndex - 1); // Stop reading before the . in front of ms
    strncpy(buffer + mk_timestampMsIndex - 1,
            utcTimestamp.c_str() + mk_timestampTimezoneIndex,
            mk_timestampTimezoneMaxLength); // Append timezone
    struct tm tm;
    strptime(buffer, "%Y-%m-%dT%H:%M:%S%z", &tm);
    int timeOffset = tm.tm_gmtoff;
    tv.tv_sec = timelocal(&tm) - timeOffset;

    // Convert milliseconds to microseconds
    char micro[10];
    memset(micro, '\0', sizeof(micro));
    // Copy last 3 characters
    strncpy(micro, utcTimestamp.c_str() + mk_timestampMsIndex, mk_timestampMsLength);
    tv.tv_usec = std::stoi(micro) * k_microToUs;
    return tv;
}
