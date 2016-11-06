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
#include "Mosquito.h"

using std::string;
using std::map;
using std::vector;

class Report {    
 public:
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
 private:

    string outputCohortFile;
    string outputAgesFile;
    string outputGroupsFile;


    std::ofstream outCohort;
    std::ofstream outAges;
    std::ofstream outGroups;
    std::ofstream outFOI;
    std::ofstream outSpatial;
    
    bool reportCohort;
    bool reportAges;
    bool reportGroups;
    bool reportFOI;
    bool reportSpatial;
    
    bool printCohortPop;
    bool printAgesPop;
    bool printGroupsPop;
    bool printGroupsAgeFirst;
    bool printGroupsTotalAges;

    vector<string> events;
    vector<string> status;

    map<string, string> parameters;

    int cohortEvents[5];
    int cohortStatus[4];
    int cohortReportPeriod[3];
    int cohortMaxIndex;
    vector<rangeStruct> cohortAges;
    vector<reportStats> cohortStats;

    int groupsEvents[5];
    int groupsStatus[2];
    int groupsReportPeriod[3];
    int groupsMaxIndex;
    int groupsAvgAgeFirst;
    int groupsTotalFirstInf;
    vector<rangeStruct> groupsAges;
    vector<reportStats> groupsStats;
    reportStats groupsTotalAgeStats;

    int ageEvents[5];
    int ageStatus[4];
    rangeStruct discreteAges;
    int ageReportPeriod[3];
    int ageMaxIndex;
    vector<reportStats> ageStats;

    int foiReportPeriod[3];
    int foiTypes[4];
    int newInfections[4];
    int susceptibles[4];
    int mozSusceptibles[4];
    int mozExposed[4];
    int mozInfectious[4];

    int spatialReportPeriod[3];
    bool spatialMosquitoes;
    vector<string> spatialData;
	
    void parseEvents(string line, int *, int);
    void parseGroupsAges(string line, vector<rangeStruct> *);
    rangeStruct parseDiscreteAges(string);
    void parsePeriod(string line, int *);
    void addParameter(string);

    void readParameter(string,string,int *);
    void readParameter(string,string, vector<rangeStruct> *);
    rangeStruct readParameter(string,string, rangeStruct);
    bool readParameter(string, bool);

    bool parseBoolean(string line);
    int getGroup(int, vector<rangeStruct>);

public:


    Report(string);
    Report();
    Report(const Report& orig);
    //virtual ~Report();

    void setupReport(string, string, string);
    void updateReport(int, Human *, Location *);
    void join(const vector<string>&,char, string &);
    void printReport(int);
    void printHeaders();
    void printGroupsHeader();
    void printCohortHeader();
    void printAgesHeader();
    void printFOIHeader();
    void printSpatialHeader();
    void printGroupsReport(int);
    void printCohortReport(int);
    void printAgesReport(int);
    void printFOIReport(int);
    void printSpatialReport(int);
    void updateCohortReport(int, Human *);
    void updateGroupsReport(int, Human *);
    void updateAgesReport(int, Human *);
    void updateFOIReport(int, Human *);
    void updateSpatialReport(int, Human *, Location *);
    void updateMosquitoReport(int, Mosquito *, Location *);
    void resetReports();
    void resetGroupStats();
    void resetCohortStats();
    void resetAgeStats();
    void resetFOIStats();
    void resetSpatialStats();
    void finalizeReport();

};

#endif	/* REPORT_H */
