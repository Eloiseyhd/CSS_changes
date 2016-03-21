#ifndef REPORT_H
#define	REPORT_H

#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <utility>
#include <memory>
#include <cmath>
#include "Human.h"



struct rangeStruct{
    int min;
    int max;
};

struct eventStats{
    int events[5];
    int nonevents[5];
};

struct reportStats{
    eventStats status[4];
    eventStats total;
};



class Report {

private:

    std::string outputCohortFile;
    std::string outputAgesFile;
    std::string outputGroupsFile;

    std::ofstream outCohort;
    std::ofstream outAges;
    std::ofstream outGroups;

    bool reportCohort;
    bool reportAges;
    bool reportGroups;

    bool printCohortPop;
    bool printAgesPop;
    bool printGroupsPop;
    bool printGroupsAgeFirst;
    bool printGroupsTotalAges;

    std::vector<std::string> events;
    std::vector<std::string> status;

    int cohortEvents[5];
    int cohortStatus[4];
    int cohortReportPeriod[3];
    int cohortMaxIndex;
    std::vector<rangeStruct> cohortAges;
    std::vector<reportStats> cohortStats;

    int groupsEvents[5];
    int groupsStatus[2];
    int groupsReportPeriod[3];
    int groupsMaxIndex;
    int groupsAvgAgeFirst;
    int groupsTotalFirstInf;
    std::vector<rangeStruct> groupsAges;
    std::vector<reportStats> groupsStats;
    reportStats groupsTotalAgeStats;

    int ageEvents[5];
    int ageStatus[4];
    std::vector<int> ages;
    int ageLB;
    int ageUB;
    int ageReportPeriod;
    int ageReportStart;
    int ageReportEnd;

    void parseEvents(std::string line, int *);
    void parseStatus(std::string line, int *);
    void parseGroupsAges(std::string line, std::vector<rangeStruct> *);
    void parsePeriod(std::string line, int *);

    bool parseComplement(std::string line);
    bool parsePrintFlag(std::string line);
    bool parseGroupsAgeFirst(std::string line);

    int getGroup(int, std::vector<rangeStruct>);

public:

    Report(std::string);
    Report();
    Report(const Report& orig);
    virtual ~Report();

    void setupReport(std::string, std::string, std::string);
    void updateReport(int, Human *);
    void printReport(int);
    void printHeaders();
    void printGroupsHeader();
    void printCohortHeader();
    void printGroupsReport(int);
    void printCohortReport(int);
    void updateCohortReport(int, Human *);
    void updateGroupsReport(int, Human *);

    void resetReports();
    void resetGroupStats();
    void resetCohortStats();
    void finalizeReport();

};

#endif	/* REPORT_H */
