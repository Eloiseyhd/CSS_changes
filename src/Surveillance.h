#ifndef SURVEILLANCE_H
#define	SURVEILLANCE_H

#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <utility>
#include <algorithm>
#include <memory>
#include <cmath>
#include "RandomNumGenerator.h"
#include "Human.h"

class Surveillance {
    struct hRecord{
	int ageDaysAtVaccination;
	int TTL[4];
	int TTR[4];
	int symptoms[4];
	int hosp[4];
	int pcrDay[4];
	int lastType;
	int dropoutDay;
	int enrollmentDay;
	int houseMemNum;
	double onset[4];
	bool seroStatusAtVaccination;
	bool previousExposure[4];
	std::string trialArm;
	std::vector<std::string> primary;
	std::vector<std::string> pcr;
	std::string houseID;
    };
 public:
    Surveillance();
    Surveillance(const Surveillance& orig);
    virtual ~Surveillance();

    void update_human_surveillance(Human *, int, RandomNumGenerator *);
    void initialize_human_surveillance(Human *, int);
    void finalize_human_surveillance(Human *, int);
    void setup(std::string);
    void printRecords(std::string, int);

 private:
    void contactPerson(Human *, int, RandomNumGenerator *);
    int contactFrequency;
    int firstContactDelay;
    double selfReportProb;
    double reportTodayProb;
    int PCR_test(Human *, int, RandomNumGenerator *);
    int parseInteger(std::string);
    double parseDouble(std::string);
    std::vector<std::string> getParamsLine(std::string);
    std::map<std::string, hRecord> recordsDatabase;
};

#endif	/* SURVEILLANCE_H */
